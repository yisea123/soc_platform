#include "dcmotor.h"
#include "fpga_dcmotor_controller.h"
#include "fpga_timer.h"
#include "fpga.h"
#include "fpga_io.h"
#include "fpgadrv.h"
#include "irqcallback.h"
#include "pwm.h"

#define FPGA_DCMOTOR_STOP_DELAY	50	//50ms

static inline int dcmotor_timer_value_get(struct fpga_dcmotor_controller_ctrl *pcontroller_ctrl, int clock_period)
{
	uint32_t value_tmp,value=0;

	value_tmp = pcontroller_ctrl->time_us*1000+pcontroller_ctrl->time_ns;
	if(value_tmp>=FPGA_TIMER_VALUE_MAX(clock_period))
	{
		value_tmp -= FPGA_TIMER_VALUE_MAX(clock_period);
		value = FPGA_TIMER_VALUE_MAX(clock_period);
		goto value_end;
	}
	else
	{
		if(!pcontroller_ctrl->time_ms)
		{
			value = value_tmp;
			value_tmp = 0;
			goto value_end;
		}
		
		pcontroller_ctrl->time_ms--;
		value_tmp += 1000000;
		value_tmp -= FPGA_TIMER_VALUE_MAX(clock_period);
		value = FPGA_TIMER_VALUE_MAX(clock_period);
	}

value_end:
	pcontroller_ctrl->time_us = value_tmp/1000;
	pcontroller_ctrl->time_ns = value_tmp%1000;
	return value;
}

static inline int dcmotor_set(struct dcmotor *motor)
{
	struct fpga_dcmotor_controller_resource *motor_controller_rc;
	struct fpga_timer *ptimer;
	struct fpga_dcmotor_controller_ctrl *pcontroller_ctrl;
	uint32_t value;
	int clock_period;
	
	if (!motor)
		return -1;		
		
	motor_controller_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	pcontroller_ctrl = &(motor_controller_rc->controller_ctrl);
	ptimer = motor_controller_rc->pfpga_dcmotor_timer;
	clock_period = ptimer->resource->clock_period;
	switch(motor_controller_rc->controller_ctrl.ctrl_status)
	{
	case DCMOTOR_CONTROLLER_RUNNING_INIT:
		value = dcmotor_timer_value_get(pcontroller_ctrl, clock_period);
		motor_controller_rc->controller_ctrl.ctrl_status = DCMOTOR_CONTROLLER_RUNNING;
		break;
	case DCMOTOR_CONTROLLER_STOP_DELAY_INIT:
		motor_controller_rc->controller_ctrl.time_ms = FPGA_DCMOTOR_STOP_DELAY;
		motor_controller_rc->controller_ctrl.time_us = 0;
		motor_controller_rc->controller_ctrl.time_ns = 0;
		value = dcmotor_timer_value_get(pcontroller_ctrl, clock_period);
		motor_controller_rc->controller_ctrl.ctrl_status = DCMOTOR_CONTROLLER_STOP_DELAY;		
		break;
	case DCMOTOR_CONTROLLER_RUNNING:
		value = dcmotor_timer_value_get(pcontroller_ctrl, clock_period);
		if(value<FPGA_TIMER_VALUE_MAX(clock_period))
		{
				motor->ops->stop(motor);
		}
		break;
	case DCMOTOR_CONTROLLER_STOP_DELAY:
		value = dcmotor_timer_value_get(pcontroller_ctrl, clock_period);
		if(value<FPGA_TIMER_VALUE_MAX(clock_period))
		{
			motor_controller_rc->controller_ctrl.ctrl_status = DCMOTOR_CONTROLLER_STOP;			
		}
		break;
	case DCMOTOR_CONTROLLER_STOP:		
		motor->ops->reset(motor);
		ptimer->ops->int_disable(ptimer);
		ptimer->ops->disable(ptimer);	
		ptimer->ops->init(ptimer);
		motor_controller_rc->controller_ctrl.ctrl_status = DCMOTOR_CONTROLLER_RUNNING_INIT;
		motor->status = 0;
		break;
	default:
		return -1;
	}

	value = TIME_VALUE_TO_TICKS(value, clock_period);	
	ptimer->ops->apr_set(ptimer, value);
	return 0;
	
}


static void fpga_dcmotor_controller_irqcallback(void *device, int id, void *data)
{
	struct dcmotor *motor = (struct dcmotor *)data;
	struct fpga_dcmotor_controller_resource *motor_controller_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	int status;
	
	fpga_clear_interrupt(motor_controller_rc->pfpga_dcmotor_timer->resource->fpga_irq_mask);
	if(motor->status == DCMOTOR_RUNNING)
	{
		dcmotor_set(motor);
	}
}

static int fpga_dcmotor_controller_start(struct dcmotor *motor)
{
	struct fpga_dcmotor_controller_resource *motor_controller_rc;
	int rs;
	struct fpga_timer *ptimer;
	struct fpga_dcmotor_controller_ctrl *pcontroller_ctrl;
	uint16_t value;
	
	if (!motor)
		return -1;	
	
	motor_controller_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	ptimer = motor_controller_rc->pfpga_dcmotor_timer;
	pcontroller_ctrl = &(motor_controller_rc->controller_ctrl);
	
	fabric_irqcallback_install(ptimer->resource->fabric_irq, (irqcallback_t)fpga_dcmotor_controller_irqcallback, (void *)motor);
	motor_controller_rc->controller_ctrl.ctrl_status = DCMOTOR_CONTROLLER_RUNNING_INIT;
	ptimer->ops->init(ptimer);
	dcmotor_set(motor);
	ptimer->ops->int_period_set(ptimer, 1);
	
	motor->status = DCMOTOR_RUNNING;
	
	rs = fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_CONTROL, FPGA_REG_DCMOTOR_CONTROLLER_RUN|FPGA_REG_DCMOTOR_CONTROLLER_PWM, 0);
	rs = fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_CONTROL, FPGA_REG_DCMOTOR_CONTROLLER_RUN, FPGA_REG_DCMOTOR_CONTROLLER_RUN);
	ptimer->ops->int_enable(ptimer);
	ptimer->ops->enable(ptimer);
	return rs;
}

struct dcmotor *pdcmotor_delay=0;


static void fpga_dcmotor_controller_stop(struct dcmotor *motor)
{
	struct fpga_dcmotor_controller_resource *motor_controller_rc;
	uint16_t value;

	if (!motor)
		return;

	motor_controller_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	pdcmotor_delay = motor;

	fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_CONTROL, FPGA_REG_DCMOTOR_CONTROLLER_PWM, FPGA_REG_DCMOTOR_CONTROLLER_PWM);
	
	motor_controller_rc->controller_ctrl.ctrl_status = DCMOTOR_CONTROLLER_STOP_DELAY_INIT;
	dcmotor_set(motor);
	
}

void	fpga_dcmotor_controller_reset(struct dcmotor *motor)
{
	struct fpga_dcmotor_controller_resource *motor_controller_rc;

	motor_controller_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_CONTROL, FPGA_REG_DCMOTOR_CONTROLLER_RUN|FPGA_REG_DCMOTOR_CONTROLLER_PWM, 0);
	
}

static int fpga_dcmotor_controller_status(struct dcmotor *motor)
{
	return motor->status;
}


static int fpga_dcmotor_controller_hw_init(struct dcmotor *motor)
{
	struct fpga_dcmotor_controller_resource *motor_controller_rc;
	uint32_t value, duty_cnt, period_cnt;
	
	motor_controller_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	// clear motor control register
	fpga_writel(0, (char *)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_CONTROL);

	period_cnt = PWM_PERIOD(motor_controller_rc, motor_controller_rc->period);
	value = BITS_11_TO_4(period_cnt);	// use only higher bit[11:4] and ignore lower bit[3:0]
	if ((period_cnt & 0xf) > 0x7)		// round-up lower bit[3:0]
		value += 1;
	fpga_writel(value, (char *)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_PERIOD);

	duty_cnt = PWM_DUTY(motor_controller_rc, motor_controller_rc->duty);

	value = BITS_11_TO_4(duty_cnt);	// use only higher bit[11:4] and ignore lower bit[3:0]
	if ((duty_cnt & 0xf) > 0x7)	// round-up lower bit[3:0]
		value += 1;
	fpga_writel(value, (char *)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_DUTY);
	
	motor_controller_rc->controller_ctrl.ctrl_status = DCMOTOR_CONTROLLER_RUNNING_INIT;
	return 0;
}

static int fpga_dcmotor_controller_config(struct dcmotor *motor, const struct dcmotor_config *config)
{
	struct fpga_dcmotor_controller_resource *motor_controller_rc;
	
	int ret, speedlevel;
	uint32_t val=0, addr;

	if (!motor || !config)
		return -1;

	if (dcmotor_is_running(motor->status))
		return -1;

	motor_controller_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	motor_controller_rc->controller_ctrl.time_ms = config->time_to_run;
	motor_controller_rc->controller_ctrl.time_us = 0;
	motor_controller_rc->controller_ctrl.time_ns = 0;
	  
	/* setup motion direction */
	val = (config->dir == MOTION_CLOCKWISE) ? FPGA_REG_DCMOTOR_CONTROLLER_DIRECTION : 0;
	addr = (uint32_t)motor_controller_rc->mmio_base + FPGA_REG_DCMOTOR_CONTROLLER_CONTROL;
	fpga_update_lbits((void *)addr, FPGA_REG_DCMOTOR_CONTROLLER_DIRECTION, val);

	return 0;
}

static struct dcmotor_ops fpga_dcmotor_controller_ops = {
	.config = fpga_dcmotor_controller_config,
	.status = fpga_dcmotor_controller_status,
	.start = fpga_dcmotor_controller_start,
	.stop = fpga_dcmotor_controller_stop,
	.reset = fpga_dcmotor_controller_reset
};

int fpga_dcmotor_controller_install(struct dcmotor *motor)
{
	struct fpga_dcmotor_controller_resource *p_motor_rc;
	int j, ret=0;
	
	if (!motor)
		return -1;
	if (!motor->resource)
		return -1;

	p_motor_rc = (struct fpga_dcmotor_controller_resource *)motor->resource;
	
	motor->ops = &fpga_dcmotor_controller_ops;

	fpga_timer_install(p_motor_rc->pfpga_dcmotor_timer);
	
	fpga_dcmotor_controller_hw_init(motor);
	return ret;
}


int fpga_dcmotor_controller_drvinit(void)
{
  	return 0;
}


