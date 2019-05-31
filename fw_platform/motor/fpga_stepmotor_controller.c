/*
 * FPGA-controlled Stepper Motor driver
 *
 * Copyright 2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "steppermotor.h"
#include "fpga_stepmotor_controller.h"
#include "fpga_timer.h"
#include "fpga.h"
#include "fpga_io.h"
#include "fpgadrv.h"
#include "steptable.h"
#include "irqcallback.h"

#define SPEED_TO_COUNT(speed,stepping,clock)	(((1000000000L / (speed)) / (stepping)) / (clock))

static void fpga_stepmotor_controller_stop(struct steppermotor *motor)
{
	struct fpga_stepmotor_controller_resource *motor_controller_rc;
	struct fpga_timer *ptimer;

	if (!motor)
		return;

	
	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	ptimer = motor_controller_rc->pfpga_stepmotor_timer;
	ptimer->ops->int_disable(ptimer);
	ptimer->ops->disable(ptimer);	
	ptimer->ops->init(ptimer);

	fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL, FPGA_REG_MOTOR_CONTROLLER_RUN, 0);
	fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL, FPGA_REG_MOTOR_CONTROLLER_EN, 0);
	motor->status = 0;
	motor_controller_rc->controller_ctrl.ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_INIT;
}

static inline int stepmotor_pulse_set(struct steppermotor *motor)
{
	int32_t i, j;
	uint32_t value;
	struct fpga_stepmotor_controller_resource *motor_controller_rc;
	struct fpga_timer *ptimer;
	struct fpga_stepmotor_controller_ctrl *pcontroller_ctrl;
	dec_ramp_attr_t dec_ramp_attr;
	
	if (!motor)
		return -1;		
		
	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	pcontroller_ctrl = &(motor_controller_rc->controller_ctrl);
		
	switch(motor_controller_rc->controller_ctrl.ctrl_status)
	{
	case STEPPERMOTOR_CONTROLLER_RUNNING_INIT:
		pcontroller_ctrl->speedtable = motor_controller_rc->rampinfo->speeds[motor_controller_rc->controller_ctrl.speed_index].accel_table;
		pcontroller_ctrl->ptr_ramp = pcontroller_ctrl->speedtable->ramp_table;
		pcontroller_ctrl->steps_count_tmp = pcontroller_ctrl->speedtable->ramp_size;
		pcontroller_ctrl->steps_count = 0;
		value = *(pcontroller_ctrl->ptr_ramp);
		pcontroller_ctrl->ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_ACC;
		break;
	case STEPPERMOTOR_CONTROLLER_RUNNING_ACC:
	  	pcontroller_ctrl->steps_count++;
		pcontroller_ctrl->steps_count_tmp--;
		pcontroller_ctrl->ptr_ramp++;
		value = *(pcontroller_ctrl->ptr_ramp);
		if(pcontroller_ctrl->steps_count_tmp==1)
		{
			pcontroller_ctrl->ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_CONST;
			pcontroller_ctrl->steps_count_tmp = pcontroller_ctrl->steps_const+1;
		}	  	
		break;
	case STEPPERMOTOR_CONTROLLER_RUNNING_CONST:
		pcontroller_ctrl->steps_count_tmp--;
		pcontroller_ctrl->steps_count++;
		if(pcontroller_ctrl->steps_count_tmp==1)
		{
			pcontroller_ctrl->ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_DEC;
			pcontroller_ctrl->speedtable = motor_controller_rc->rampinfo->speeds[pcontroller_ctrl->speed_index].decel_table;
			dec_ramp_attr = pcontroller_ctrl->speedtable->dec_ramp_attr;
			pcontroller_ctrl->ptr_ramp = pcontroller_ctrl->speedtable->ramp_table;
			pcontroller_ctrl->steps_count_tmp = pcontroller_ctrl->speedtable->ramp_size;
			if(dec_ramp_attr == DEC_RAMP_REVERSED)
				pcontroller_ctrl->ptr_ramp = pcontroller_ctrl->speedtable->ramp_table+pcontroller_ctrl->steps_count_tmp-1;
		}
		return 0;
	case STEPPERMOTOR_CONTROLLER_RUNNING_DEC:
		motor_controller_rc->controller_ctrl.steps_count_tmp--;
		pcontroller_ctrl->steps_count++;
		dec_ramp_attr = motor_controller_rc->controller_ctrl.speedtable->dec_ramp_attr;
		if(dec_ramp_attr == DEC_RAMP_REVERSED)		
		  	motor_controller_rc->controller_ctrl.ptr_ramp--;
		else
		  	motor_controller_rc->controller_ctrl.ptr_ramp++;
		value = *(motor_controller_rc->controller_ctrl.ptr_ramp);
		if(motor_controller_rc->controller_ctrl.steps_count_tmp==1)
		{
			motor_controller_rc->controller_ctrl.ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_HOLD;
		}
		break;
	case STEPPERMOTOR_CONTROLLER_RUNNING_HOLD:
		motor_controller_rc->controller_ctrl.ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_INIT;
		fpga_stepmotor_controller_stop(motor);
		return 0;
	default:
		return -1;
	}

	ptimer = motor_controller_rc->pfpga_stepmotor_timer;

	value = TIME_VALUE_TO_TICKS(value, ptimer->resource->clock_period);
	
	ptimer->ops->apr_set(ptimer, value);
	return 0;
	
}

static void fpga_stepmotor_controller_irqcallback(void *device, int id, void *data)
{
	struct steppermotor *motor = (struct steppermotor *)data;
	struct fpga_stepmotor_controller_resource *motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	int status;
	
	fpga_clear_interrupt(motor_controller_rc->pfpga_stepmotor_timer->resource->fpga_irq_mask);
	if(motor->status == STEPPERMOTOR_RUNNING)
	{
		stepmotor_pulse_set(motor);
	}
}

static int fpga_stepmotor_controller_start(struct steppermotor *motor)
{
	struct fpga_stepmotor_controller_resource *motor_controller_rc;
	int rs;
	struct fpga_timer *ptimer;
	struct fpga_stepmotor_controller_ctrl *pcontroller_ctrl;
	
	if (!motor)
		return -1;	
	
	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	ptimer = motor_controller_rc->pfpga_stepmotor_timer;
	pcontroller_ctrl = &(motor_controller_rc->controller_ctrl);

	
	fabric_irqcallback_install(ptimer->resource->fabric_irq, (irqcallback_t)fpga_stepmotor_controller_irqcallback, (void *)motor);

	motor_controller_rc->controller_ctrl.ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_INIT;
	
	ptimer->ops->init(ptimer);
	stepmotor_pulse_set(motor);
	ptimer->ops->int_period_set(ptimer, 1);
	
	motor->status = STEPPERMOTOR_RUNNING;

	
	rs = fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL, FPGA_REG_MOTOR_CONTROLLER_EN, FPGA_REG_MOTOR_CONTROLLER_EN);
	rs = fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL, FPGA_REG_MOTOR_CONTROLLER_RUN, 0);
	rs = fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL, FPGA_REG_MOTOR_CONTROLLER_RUN, FPGA_REG_MOTOR_CONTROLLER_RUN);
	ptimer->ops->int_enable(ptimer);
	ptimer->ops->enable(ptimer);
	return rs;
}

static void fpga_stepmotor_controller_reset(struct steppermotor *motor)
{
  
}


static void fpga_stepmotor_controller_softstop(struct steppermotor *motor)
{
	struct fpga_stepmotor_controller_resource *motor_controller_rc;
	struct fpga_stepmotor_controller_ctrl *pcontroller_ctrl;
	dec_ramp_attr_t dec_ramp_attr;
	
	if (!motor)
		return;		
		
	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	pcontroller_ctrl = &(motor_controller_rc->controller_ctrl);

	if(pcontroller_ctrl->ctrl_status<STEPPERMOTOR_CONTROLLER_RUNNING_DEC)
	{
		pcontroller_ctrl->ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_DEC;
		pcontroller_ctrl->speedtable = motor_controller_rc->rampinfo->speeds[pcontroller_ctrl->speed_index].decel_table;
		dec_ramp_attr = pcontroller_ctrl->speedtable->dec_ramp_attr;
		pcontroller_ctrl->ptr_ramp = pcontroller_ctrl->speedtable->ramp_table;
		pcontroller_ctrl->steps_count_tmp = pcontroller_ctrl->speedtable->ramp_size;
		if(dec_ramp_attr == DEC_RAMP_REVERSED)
			pcontroller_ctrl->ptr_ramp = pcontroller_ctrl->speedtable->ramp_table+pcontroller_ctrl->steps_count_tmp-1;
	}
}


static int fpga_stepmotor_controller_status(struct steppermotor *motor)
{
	return motor->status;
}


static int fpga_stepmotor_controller_hw_init(struct steppermotor *motor)
{
	struct fpga_stepmotor_controller_resource *motor_controller_rc;

	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	// clear motor control register
	fpga_writel(0, (char *)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL);
	motor_controller_rc->controller_ctrl.ctrl_status = STEPPERMOTOR_CONTROLLER_RUNNING_INIT;
	return 0;
}

static int fpga_stepmotor_controller_config(struct steppermotor *motor, const struct steppermotor_config *config)
{
	struct fpga_stepmotor_controller_resource *motor_controller_rc;
	
	int ret, speedlevel;
	uint32_t val=0;

	if (!motor || !config)
		return -1;

	ret = steppermotor_check_config(motor, config);
	if (ret < 0)
	{
		printf("error in configuration of steppermotor");
		return -ret;
	}

	if (steppermotor_is_running(motor->status))
		return -1;

	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;

	if((motor_controller_rc->select==0)||(motor_controller_rc->select==1))
		ret = fpga_update_lbits((char *)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL, FPGA_REG_MOTOR_CONTROLLER_SEL, FPGA_MOTOR_CONTROLLER_SEL(motor_controller_rc->select));
	else
		return -1;

	motor_controller_rc->controller_ctrl.speedinfo = config->speedinfo;
	motor_controller_rc->controller_ctrl.steps_to_run = config->speedinfo->steps* motor_controller_rc->stepping_mode;
	
	motor_controller_rc->controller_ctrl.speed_index = lookup_speedtable(motor, config->speedinfo->speed);
	motor_controller_rc->controller_ctrl.steps_const = motor_controller_rc->controller_ctrl.steps_to_run;
	motor_controller_rc->controller_ctrl.steps_const -= motor_controller_rc->rampinfo->speeds[motor_controller_rc->controller_ctrl.speed_index].accel_table->ramp_size;
	motor_controller_rc->controller_ctrl.steps_const -= motor_controller_rc->rampinfo->speeds[motor_controller_rc->controller_ctrl.speed_index].decel_table->ramp_size;
	
	switch(motor_controller_rc->stepping_mode)
	{
	case STEP_MODE_FULL:
		val = 0;
		break;
	case STEP_MODE_HALF:
		val = 2;
		break;
	case STEP_MODE_QUARTER:
		val = 3;
		break;
	case STEP_MODE_8MICRO:
		val = 4;
		break;
	case STEP_MODE_16MICRO:
		val = 5;	//for driver 8880
		//val = 4;	//for driver 8825
		break;
	}	
	fpga_update_lbits((void *)((uint32_t)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL), FPGA_REG_MOTOR_CONTROLLER_MODE, FPGA_MOTOR_CONTROLLER_MODE(val));
	
	/* setup motion direction */
	val = (config->dir == MOTION_CLOCKWISE) ? FPGA_REG_MOTOR_CONTROLLER_DIRECTION : 0;
	fpga_update_lbits((void *)((uint32_t)motor_controller_rc->mmio_base + FPGA_REG_MOTOR_CONTROLLER_CONTROL), FPGA_REG_MOTOR_CONTROLLER_DIRECTION, val);

	return 0;
}

static int fpga_stepmotor_controller_get_running_steps(struct steppermotor *motor)
{
	struct fpga_stepmotor_controller_resource *motor_controller_rc;
	uint32_t val;
	int rs;

	if (!motor)
		return -1;

	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	val = motor_controller_rc->controller_ctrl.steps_count;
	val = val / motor_controller_rc->stepping_mode;
	return val;
}

static int fpag_stepmotor_controller_set_running_steps(struct steppermotor *motor, int steps)
{
  	struct fpga_stepmotor_controller_resource *motor_controller_rc;
	uint32_t steps_const, steps_dec;
	int rs;

	if (!motor)
		return -1;
	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	if (!steppermotor_is_running(motor->status))
		return -1;

	if(motor_controller_rc->controller_ctrl.ctrl_status!=STEPPERMOTOR_CONTROLLER_RUNNING_CONST)
		return -1;
	motor_controller_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	
  	steps_const = motor_controller_rc->controller_ctrl.steps_const;
	steps_dec = motor_controller_rc->rampinfo->speeds[motor_controller_rc->controller_ctrl.speed_index].decel_table->ramp_size;
	
	steps *= motor_controller_rc->stepping_mode;
	if(steps>steps_dec)
	{
		steps -= steps_dec;

		motor_controller_rc->controller_ctrl.steps_const = steps;
		motor_controller_rc->controller_ctrl.steps_count_tmp = steps;
	}
	else
	{
		fpga_stepmotor_controller_softstop(motor);
	}
	return 0;
}

static struct steppermotor_ops fpga_stepmotor_controller_ops = {
	.config = fpga_stepmotor_controller_config,
	.status = fpga_stepmotor_controller_status,
	.start = fpga_stepmotor_controller_start,
	.reset = fpga_stepmotor_controller_reset,
	.stop = fpga_stepmotor_controller_softstop,
	.get_running_steps = fpga_stepmotor_controller_get_running_steps,
	.set_running_steps = fpag_stepmotor_controller_set_running_steps,
};

int fpga_stepmotor_controller_install(struct steppermotor *motor)
{
	struct fpga_stepmotor_controller_resource *p_motor_rc;
	int j, ret=0;
	
	if (!motor)
		return -1;
	if (!motor->resource)
		return -1;

	p_motor_rc = (struct fpga_stepmotor_controller_resource *)motor->resource;
	
	motor->ops = &fpga_stepmotor_controller_ops;

	fpga_timer_install(p_motor_rc->pfpga_stepmotor_timer);

	motor->feature.max_steps = MAXIMUM_STEPS / p_motor_rc->stepping_mode;  
	motor->feature.pullin_speed = p_motor_rc->pullin_speed;
	motor->feature.num_speed = p_motor_rc->rampinfo->num_speed;
		
	for(j=0; j<motor->feature.num_speed; j++)
	{
		motor->feature.speeds[j].speed = p_motor_rc->rampinfo->speeds[j].accel_table->object_speed;
		motor->feature.speeds[j].accel_steps = p_motor_rc->rampinfo->speeds[j].accel_table->ramp_size/p_motor_rc->rampinfo->speeds[j].accel_table->stepping;
		motor->feature.speeds[j].decel_steps = p_motor_rc->rampinfo->speeds[j].decel_table->ramp_size/p_motor_rc->rampinfo->speeds[j].decel_table->stepping;
	}
		
	fpga_stepmotor_controller_hw_init(motor);
	return ret;
}


int fpga_stepmotor_controller_drvinit(void)
{
  	return 0;
}




