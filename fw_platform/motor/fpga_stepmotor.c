/*
 * FPGA-controlled Stepper Motor driver
 *
 * Copyright 2016-2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "steppermotor.h"
#include "fpga_stepmotor.h"
#include "fpga.h"
#include "fpga_io.h"
#include "fpgadrv.h"
#include "steptable.h"
#include "irqcallback.h"

   
#define SPEED_TO_COUNT(speed,stepping,clock)	(((1000000000L / (speed)) / (stepping)) / (clock))

#define MOTOR_TABLE_RAMP_LIMIT		((void *)((uint32_t)motor_rc->ram_base + motor_rc->ram_ramp_offset + motor_rc->table_ramp_size))
#define MOTOR_TABLE_COUNT_LIMIT		((void *)((uint32_t)motor_rc->mmio_base + FPGA_REG_MOTOR_ACCEL_STEPS + motor_rc->table_count_size))


static inline int _fpga_stepmotor_status(struct steppermotor *motor);

static void fpga_stepmotor_irqcallback(void *device, int id, void *data)
{
	struct steppermotor *motor = (struct steppermotor *)data;
	struct fpga_stepmotor_resource *motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	int status;
	
	fpga_clear_interrupt(motor_rc->fpga_irq_mask);
	status = _fpga_stepmotor_status(motor);
	if(status == STEPPERMOTOR_RUNNING)
	{
		fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN, 0);
		motor->status = STEPPERMOTOR_STOPPED_BY_TOTAL_STEPS;
		if(motor->callback)
			motor->callback(motor, &motor->callbackdata);
	}
}


static int fpga_stepmotor_start(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;
	int rs;

	if (!motor)
		return -1;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
          
    fpga_enable_interrupt(motor_rc->fpga_irq_mask);  
	rs = fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, 0);
	rs = fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN, FPGA_REG_MOTOR_RUN);
	motor->status = STEPPERMOTOR_RUNNING;
	return rs;
}


static void fpga_stepmotor_stop(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;

	if (!motor)
		return;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP, FPGA_REG_MOTOR_STOP);
}


static void fpga_stepmotor_emergencybrake(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;

	if (!motor)
		return;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, FPGA_REG_MOTOR_EMERGENCY_BRAKE);

	//motor->status = _fpga_stepmotor_status(motor);
	motor->status = STEPPERMOTOR_STOPPED_BY_TOTAL_STEPS;
}

#if 0
static inline int _fpga_stepmotor_status(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;
	int rs, ret;
	uint32_t status;

	if (!motor)
		return -1;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	rs = fpga_readl(&status, (char *)motor_rc->mmio_base + FPGA_REG_MOTOR_STATUS);
	if (rs)
		return 0;
	ret = 0;
	return ret;
}
#else
static inline int _fpga_stepmotor_status(struct steppermotor *motor)
{
  	return motor->status;
}
#endif

static int fpga_stepmotor_status(struct steppermotor *motor)
{
	return _fpga_stepmotor_status(motor);
}


static inline void fpga_ram_clear(void *addr, int size)
{
	int i;
	for (i = 0; i < size;)
	{
		fpga_writel(0, addr);
		addr = (void *)((uint32_t)addr + sizeof(uint32_t));
		i += sizeof(uint32_t);
	}
}


static int fpga_stepmotor_hw_init(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	// clear motor control register
	fpga_writel(0, (char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL);

	// clear entire RAM of this steppermotor
	fpga_ram_clear(motor_rc->ram_base, motor_rc->ram_size);

	return 0;
}


static inline void  * fpga_ram_load_ramptable(void *addr, void *limit, uint32_t flag, struct motor_speedtable *speedtable)
{
	void  *ptr = addr;
	int32_t i;
	uint32_t val, val1;
	dec_ramp_attr_t dec_ramp_attr = speedtable->dec_ramp_attr;

	flag &= ~0x0f;
		
	if(dec_ramp_attr == DEC_RAMP_REVERSED)
	{
		for (i = 0; i < speedtable->ramp_size; i++) 
		{
			if (ptr >= limit) {
				return (void  *)(-1);	// exceed table limit, return error
			}
			val = speedtable->ramp_table[speedtable->ramp_size-i-1] | flag;
			fpga_writel(val, ptr);
			fpga_readl(&val1, ptr);//test

			ptr = (void *)((uint32_t)ptr+sizeof(uint32_t));
		}
	}
	else
	{
		for (i = 0; i < speedtable->ramp_size; i++) 
		{
			if (ptr >= limit) {
				return (void  *)(-1);	// exceed table limit, return error
			}
			val = speedtable->ramp_table[i] | flag;
			fpga_writel(val, ptr);
			fpga_readl(&val1, ptr);//test
			ptr = (void *)((uint32_t)ptr+sizeof(uint32_t));
		}
	}
	return ptr;		// return next loading address
}


static inline void * fpga_ram_load_value(void *addr, uint32_t value)
{
	fpga_writel(value, addr);
	addr = (void *)((uint32_t)addr + sizeof(uint32_t));
	return addr;		// return next loading address
}


static int fpga_stepmotor_config(struct steppermotor *motor, const struct steppermotor_config *config)
{
	struct fpga_stepmotor_resource *motor_rc;
	struct speed_info *speedinfo;
	int ret, steps, speedlevel;
	uint32_t val;

	if (!motor || !config)
		return -1;

	ret = steppermotor_check_config(motor, config);
	if (ret < 0)
	{
//		dev_err(motor->dev, "error in configuration of steppermotor");
		return -ret;
	}

	if (steppermotor_is_running(motor->status))
		return -1;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;

	// clear ramp table and speed profile table
	fpga_ram_clear((char *)motor_rc->ram_base + motor_rc->ram_ramp_offset, motor_rc->table_ramp_size); 
	fpga_ram_clear((void *)((uint32_t)motor_rc->mmio_base + FPGA_REG_MOTOR_ACCEL_STEPS), motor_rc->table_count_size);


	if (config->steps_to_run == 1)	// single-step configuration
	{
		/* write speed table */
		val = SPEED_TO_COUNT(motor->feature.pullin_speed, motor_rc->stepping, motor_rc->clock_period);
		val |= FPGA_RAM_MOTOR_TABLE_RAMP_CONST1;
		ret = fpga_writel(val, (void *)((uint32_t)motor_rc->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP));

		/* write profile table */
		val = 1 * motor_rc->stepping;
		ret = fpga_writel(val, (void *)((uint32_t)motor_rc->mmio_base + FPGA_REG_MOTOR_ACCEL_STEPS)); 
	}
	else	// multiple-steps configuration
	{
		struct motor_speedtable *speedtable;
		void *addr, *ptr_ramp, *ptr_count;
		int index, rsteps;
		int speed1;

		speedinfo = config->speedinfo;
		steps = speedinfo->steps;
		speed1 = speedinfo->speed;

		// compose single-speed or multiple-speeds and load int RAM
		index = lookup_speedtable(motor, speed1);
		speedtable = motor_rc->rampinfo->speeds[index].accel_table;

		rsteps = steps * speedtable->stepping;

		ptr_ramp = (void *)((uint32_t)motor_rc->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP);
		ptr_count = (void *)((uint32_t)motor_rc->mmio_base + FPGA_REG_MOTOR_ACCEL_STEPS);

		// load acceleration table of speed 1
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_RAMP_LIMIT, FPGA_RAM_MOTOR_TABLE_RAMP_ACCEL, speedtable); 
		if (addr<0)
			goto err_handling;
		else
			ptr_ramp = addr;


		// setup acceleration steps
		ptr_count = fpga_ram_load_value(ptr_count, speedtable->ramp_size);
		rsteps -= speedtable->ramp_size;

		speedtable = motor_rc->rampinfo->speeds[index].decel_table;

		// load constant-speed table entry of speed 1
		if(speedtable->dec_ramp_attr != DEC_RAMP_REVERSED)
			val = speedtable->ramp_table[0] | FPGA_RAM_MOTOR_TABLE_RAMP_CONST1; 
		else
			val = speedtable->ramp_table[speedtable->ramp_size-1] | FPGA_RAM_MOTOR_TABLE_RAMP_CONST1; 
                
		ptr_ramp = fpga_ram_load_value(ptr_ramp, val);
		speedlevel = motor_rc->rampinfo->speeds[index].step_ticks; 

		if (config->num_speed == 1)	// single speed configuration
		{
			// setup constant-speed steps
			rsteps = rsteps - speedtable->ramp_size;
			ptr_count = fpga_ram_load_value(ptr_count, rsteps);
			// setup deceleration steps
			ptr_count = fpga_ram_load_value(ptr_count, speedtable->ramp_size);
		}
		else	// speed shift configuration
		{			

		}
		// load deceleration ramp table
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_RAMP_LIMIT, FPGA_RAM_MOTOR_TABLE_RAMP_DECEL, speedtable);
err_handling:
	}

	/* setup motion direction */
	val = (config->dir == MOTION_CLOCKWISE) ? FPGA_REG_MOTOR_DIRECTION : 0;
	fpga_update_lbits((void *)((uint32_t)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_DIRECTION, val);

	return 0;
}


static int fpga_stepmotor_get_running_steps(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;
	uint32_t val;
	int rs;

	if (!motor)
		return -1;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;

	rs = fpga_readl(&val, (char *)motor_rc->mmio_base + FPGA_REG_MOTOR_RUNNING_STEPS);
	if (rs)
		return rs;

	val = val / motor_rc->stepping;
	return val;
}



static struct steppermotor_ops fpga_stepmotor_ops = {
	.config = fpga_stepmotor_config,
	.status = fpga_stepmotor_status,
	.start = fpga_stepmotor_start,
	.stop = fpga_stepmotor_stop,
	.emergencybrake = fpga_stepmotor_emergencybrake,
	.get_running_steps = fpga_stepmotor_get_running_steps,
};

int fpga_stepmotor_install(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *p_motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	int j, ret=0;
	
	if (!motor)
		return -1;
	if (!motor->resource)
		return -1;

	motor->ops = &fpga_stepmotor_ops;

	fabric_irqcallback_install(p_motor_rc->fabric_irq, (irqcallback_t)fpga_stepmotor_irqcallback, (void *)motor);

	motor->feature.max_steps = MAXIMUM_STEPS / p_motor_rc->stepping; 
	motor->feature.pullin_speed = p_motor_rc->pullin_speed;
	motor->feature.num_speed = p_motor_rc->rampinfo->num_speed;
		
	for(j=0; j<motor->feature.num_speed; j++)
	{
		motor->feature.speeds[j].speed = p_motor_rc->rampinfo->speeds[j].accel_table->object_speed;
		motor->feature.speeds[j].accel_steps = p_motor_rc->rampinfo->speeds[j].accel_table->ramp_size/p_motor_rc->rampinfo->speeds[j].accel_table->stepping;
		motor->feature.speeds[j].decel_steps = p_motor_rc->rampinfo->speeds[j].decel_table->ramp_size/p_motor_rc->rampinfo->speeds[j].decel_table->stepping;
		if(p_motor_rc->rampinfo->speeds[j].accel_table->ramp_size > p_motor_rc->ram_size)
		{
			return -1;
		}
		if(p_motor_rc->rampinfo->speeds[j].decel_table->ramp_size > p_motor_rc->ram_size)
		{
			return -1;
		}
	}
		
	ret = steppermotor_ramptable_convert(p_motor_rc->rampinfo, p_motor_rc->clock_period); 
	
	fpga_stepmotor_hw_init(motor);
	return ret;
}


int fpga_stepmotor_drvinit(void)
{
  	return 0;
}


