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

static inline int _fpga_stepmotor_status(struct steppermotor *motor);


static int fpga_stepmotor_start(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;
	int rs;

	if (!motor)
		return -1;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	rs = fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, 0);
	rs = fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN, FPGA_REG_MOTOR_RUN);

	motor->status |= STEPPERMOTOR_RUNNING;
	return rs;
}


static void fpga_stepmotor_stop(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;

	if (!motor)
		return;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, FPGA_REG_MOTOR_STOP);
}


static void fpga_stepmotor_emergencybrake(struct steppermotor *motor)
{
	struct fpga_stepmotor_resource *motor_rc;

	if (!motor)
		return;

	motor_rc = (struct fpga_stepmotor_resource *)motor->resource;
	fpga_update_lbits((char *)motor_rc->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, FPGA_REG_MOTOR_EMERGENCY_BRAKE);

	motor->status = _fpga_stepmotor_status(motor);
}


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
//		((char *)addr) += sizeof(uint32_t);
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
	return NULL;		// return next loading address
}


static inline void * fpga_ram_load_value(void *addr, uint32_t value)
{
	fpga_writel(value, addr);
//	addr += sizeof(u32);
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
	fpga_ram_clear((char *)motor_rc->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP, FPGA_RAM_MOTOR_TABLE_RAMP_SIZE); 
	fpga_ram_clear((char *)motor_rc->ram_base + FPGA_RAM_MOTOR_TABLE_COUNT, FPGA_RAM_MOTOR_TABLE_COUNT_SIZE);

#if 0	// TODO: 请修改以下代码!!!

	if (config->steps_to_run == 1)	// single-step configuration
	{
		/* write speed table */
		val = SPEED_TO_COUNT(motor->feature.pullin_speed, motordev->stepping, motordev->clock_period);
		val |= FPGA_RAM_MOTOR_TABLE_RAMP_CONST1;
		ret = fpga_writel(val, motordev->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP);

		/* write profile table */
		val = 1 * motordev->stepping;
		ret = fpga_writel(val, motordev->ram_base + FPGA_RAM_MOTOR_TABLE_COUNT); 
	}
	else	// multiple-steps configuration
	{
		struct motor_speedtable *speedtable, *stoptable_low, *stoptable_high;
		void __iomem *addr, *ptr_ramp, *ptr_count;
		int index, rsteps;
		int speed1, speed2;

		speedinfo = config->speedinfo;
		steps = speedinfo->steps;
		speed1 = speedinfo->speed;

		// compose single-speed or multiple-speeds and load int RAM
		index = lookup_speedtable(motor, speed1);
		speedtable = motordev->rampinfo.speeds[index].accel_table;

		rsteps = steps * speedtable->stepping;

		ptr_ramp = motordev->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP;
		ptr_count = motordev->ram_base + FPGA_RAM_MOTOR_TABLE_COUNT;

		// load acceleration table of speed 1
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_RAMP_LIMIT, FPGA_RAM_MOTOR_TABLE_RAMP_ACCEL, speedtable); 
		dev_dbg(motor->dev, "load acceleration ramp table (speed1) @ %08x, ret=%08x", (u32)ptr_ramp, (u32)addr);
		if (IS_ERR(addr))
			goto err_handling;
		else
			ptr_ramp = addr;


		// setup acceleration steps
		ptr_count = fpga_ram_load_value(ptr_count, speedtable->ramp_size);
		rsteps -= speedtable->ramp_size;

		speedtable = stoptable_low = stoptable_high = motordev->rampinfo.speeds[index].decel_table;

		// load constant-speed table entry of speed 1
		val = speedtable->ramp_table[0] | FPGA_RAM_MOTOR_TABLE_RAMP_CONST1; 
		ptr_ramp = fpga_ram_load_value(ptr_ramp, val);
		speedlevel = motordev->rampinfo.speeds[index].step_ticks; 

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
			struct motor_speedtable *shifttable;
			int flag, rsteps2;

			speedinfo = speedinfo->nextspeed;
			speed2 = speedinfo->speed;
			// get speed-shift table
			index = lookup_shifttable(motor, speed1, speed2);
			shifttable = motordev->rampinfo.speedshifts[index].shift_table;

			// get deceleration ramp table of speed 2
			index = lookup_speedtable(motor, speed2);
			speedtable = motordev->rampinfo.speeds[index].decel_table;

			rsteps2 = speedinfo->steps * speedtable->stepping;

			// setup stop tables
			if (speed1 < speed2)
				stoptable_high = speedtable;
			else
			{
				stoptable_low = speedtable;
				speedlevel = speedtable->ramp_table[0];		// set level to lower speed
			}

			// setup constant-speed steps of speed 1
			ptr_count = fpga_ram_load_value(ptr_count, rsteps);
			// setup speed-shift steps
			ptr_count = fpga_ram_load_value(ptr_count, shifttable->ramp_size);
			val = rsteps2 - shifttable->ramp_size - speedtable->ramp_size;
			// setup constant-speed steps of speed 2
			ptr_count = fpga_ram_load_value(ptr_count, val);
			// setup deceleration steps of speed 2
			ptr_count = fpga_ram_load_value(ptr_count, speedtable->ramp_size);

			// load speed-shift ramp table
			flag = (speed1 < speed2) ? FPGA_RAM_MOTOR_TABLE_RAMP_ACCEL : FPGA_RAM_MOTOR_TABLE_RAMP_DECEL;
			addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_RAMP_LIMIT, flag, shifttable);
			dev_dbg(motor->dev, "load speed-shift ramp table @ %08x, ret=%08x", (u32)ptr_ramp, (u32)addr);
			if (IS_ERR(addr))
				goto err_handling;
			else
				ptr_ramp = addr;

			// load constant-speed table entry of speed 2
			val = speedtable->ramp_table[0] | FPGA_RAM_MOTOR_TABLE_RAMP_CONST2; 
			ptr_ramp = fpga_ram_load_value(ptr_ramp, val);

		}
		// load deceleration ramp table
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_RAMP_LIMIT, FPGA_RAM_MOTOR_TABLE_RAMP_DECEL, speedtable);
		dev_dbg(motor->dev, "load deceleration ramp table @ %08x, ret=%08x", (u32)ptr_ramp, (u32)addr);
err_handling:

		// load stop tables
		ptr_ramp = motordev->ram_base + FPGA_RAM_MOTOR_TABLE_STOPHIGH;
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_STOPHIGH_LIMIT, 0, stoptable_high);
		dev_dbg(motor->dev, "load stop table HIGH @ %08x, ret=%08x", (u32)ptr_ramp, (u32)addr);

		ptr_ramp = motordev->ram_base + FPGA_RAM_MOTOR_TABLE_STOPLOW;
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_STOPLOW_LIMIT, 0, stoptable_low);
		dev_dbg(motor->dev, "load stop tables LOW @ %08x, ret=%08x", (u32)ptr_ramp, (u32)addr);
	}

	/* setup motion direction */
	val = (config->dir == MOTION_CLOCKWISE) ? FPGA_REG_MOTOR_DIRECTION : 0;
	fpga_update_lbits(motordev->mmio_base + FPGA_REG_MOTOR_CONTROL, FPGA_REG_MOTOR_DIRECTION, val);

	/* setup speed level register */
	fpga_writel(speedlevel, motordev->mmio_base + FPGA_REG_MOTOR_SPEEDLEVEL);

	/* setup preset running steps register */
	val = config->steps_to_run * motordev->stepping;
	fpga_writel(val, motordev->mmio_base + FPGA_REG_MOTOR_PRESET_STEPS);

#endif // end of TODO

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
	if (!motor)
		return -1;
	if (!motor->resource)
		return -1;

	motor->ops = &fpga_stepmotor_ops;
	return 0;
}


int fpga_stepmotor_drvinit(void)
{
  	return 0;
}


#if 0	// TODO: 以下是原来的代码，请将以上实现函数作相应修改


extern int32_t motorint_handler_register(uint32_t mask, irq_handler_t handler, void * dev_id);
extern int32_t motorint_handler_unregister(irq_handler_t handler);

//static __INLINE int _fpga_stepmotor_status(struct stepmotor_dev *motordev);




#define SPEED_TO_COUNT(speed,stepping,clock)	(((1000000000L / (speed)) / (stepping)) / (clock))

#if 0
#define TRIGGER_NEXT_CTRL_MASK	(FPGA_REG_MOTOR_STOP_AT_TRGSTEP_END | FPGA_REG_MOTOR_SENSOR_CHECK_MODE |	\
				FPGA_REG_MOTOR_SENSER_STOP_ENABLE | FPGA_REG_MOTOR_SENSER_CONTINUE_MODE |	\
				FPGA_REG_MOTOR_SENSER_STOP_MODE )
#endif
#define MOTOR_TABLE_RAMP_LIMIT		((void *)((uint32_t)motordev->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP + FPGA_RAM_MOTOR_TABLE_RAMP_SIZE))
#define MOTOR_TABLE_COUNT_LIMIT		((void *)((uint32_t)motordev->mmio_base + FPGA_RAM_MOTOR_TABLE_COUNT + FPGA_RAM_MOTOR_TABLE_COUNT_SIZE))
//#define MOTOR_TABLE_STOPHIGH_LIMIT	((void *)((uint32_t)motordev->ram_base + FPGA_RAM_MOTOR_TABLE_STOPHIGH + FPGA_RAM_MOTOR_TABLE_STOPHIGH_SIZE))
//#define MOTOR_TABLE_STOPLOW_LIMIT	((void *)((uint32_t)motordev->ram_base + FPGA_RAM_MOTOR_TABLE_STOPLOW + FPGA_RAM_MOTOR_TABLE_STOPLOW_SIZE))

static int32_t fpga_stepmotor_start(struct stepmotor_dev *motordev)
{
	int32_t rs;

	if (!motordev)
		return -EINVAL;

	#if 0
	rs = fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, 0);
	#else
	rs = fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP, 0);
	#endif
	rs = fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_RUN, FPGA_REG_MOTOR_RUN);

	motordev->pmotor->status |= STEPPERMOTOR_RUNNING;
	return rs;
}


static void fpga_stepmotor_stop(struct stepmotor_dev *motordev)
{
	if (!motordev)
		return;
	#if 0
	fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, FPGA_REG_MOTOR_STOP);	
	#else
	fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP, FPGA_REG_MOTOR_STOP);	
	#endif
}

#if 0

static void fpga_stepmotor_emergencybrake(struct stepmotor_dev *motordev)
{
	if (!motordev)
		return;

	fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_RUN|FPGA_REG_MOTOR_STOP|FPGA_REG_MOTOR_EMERGENCY_BRAKE, FPGA_REG_MOTOR_EMERGENCY_BRAKE);

	motordev->pmotor->status = _fpga_stepmotor_status(motordev);
}


int32_t rs_temp[10], rs_count=0;
static __INLINE int32_t _fpga_stepmotor_status(struct stepmotor_dev *motordev)
{
	int32_t rs, ret;
	uint32_t status;

	if (!motordev)
		return -EINVAL;

	rs = fpga_readl(&status, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_INTERRUPT_FLAG));
	if (rs)
		return 0;

	rs_temp[rs_count++] = status;
	ret = 0;
	if (status & FPGA_REG_MOTOR_STOPPED)
	{
		if (status & FPGA_REG_MOTOR_STOPPED_BY_CONDITION) 
			ret |= STEPPERMOTOR_STOPPED_BY_SENSOR;
		else if (status & FPGA_REG_MOTOR_STOPPED_BY_TRIGGERSTEP) 
			ret |= STEPPERMOTOR_STOPPED_BY_TRIGGER;
		else
			ret |= STEPPERMOTOR_STOPPED_BY_TOTAL_STEPS;
	}
	else
		ret |= STEPPERMOTOR_RUNNING;

	if (status & FPGA_REG_MOTOR_TRIGGERSTEP_DONE)
		ret |= STEPPERMOTOR_TRIGGER_STEPS_DONE;
	if (status & FPGA_REG_MOTOR_FAULT)
		ret |= STEPPERMOTOR_FAULT;

	return ret;
}


static int32_t fpga_stepmotor_status(struct stepmotor_dev *motordev)
{
	return _fpga_stepmotor_status(motordev);
}
#else
static int32_t fpga_stepmotor_status(struct stepmotor_dev *motordev)
{
	return motordev->pmotor->status;
}
#endif


static __INLINE void fpga_ram_clear(void  *addr, uint32_t size)
{
	uint32_t i;
	
	for (i = 0; i < size;)
	{		
		fpga_writel(0, addr);
		addr = (void *)((uint32_t)addr + sizeof(uint32_t));
		i += sizeof(uint32_t);
		
		if(!(i%100))
		//if(i==(0x1000-1))
			fpga_writel(0, addr);
	}
}


int32_t fpga_stepmotor_hw_init(struct stepmotor_dev *motordev)
{
	int32_t ret;

	// clear motor control register
	ret = fpga_writel(0, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL));

	// clear entire RAM of this steppermotor
	fpga_ram_clear(motordev->ram_base, motordev->ram_size);

	return 0;
}


static __INLINE void  * fpga_ram_load_ramptable(void  *addr, void  *limit, uint32_t flag, struct motor_speedtable *speedtable)
{
	void  *ptr = addr;
	int32_t i;
	uint32_t val, val1;
	dec_ramp_attr_t dec_ramp_attr = flag & 0x0f;

	flag &= ~0x0f;
		
	if(dec_ramp_attr == DEC_RAMP_REVERSED)
	{
		ptr = (void *)((uint32_t)ptr+sizeof(uint32_t)*speedtable->ramp_size);
		for (i = 0; i < speedtable->ramp_size; i++) 
		{
			if (ptr >= limit) {
				//printk(KERN_ERR "fpga_stepmoter: ramp table [@%08x] exceeds steppermotor RAM limit %08x\n",
				//	(u32)ptr, (u32)limit);
				return (void  *)(-EINVAL);	// exceed table limit, return error
			}
			val = speedtable->pramp_table[speedtable->ramp_size-i-1] | flag;
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
				//printk(KERN_ERR "fpga_stepmoter: ramp table [@%08x] exceeds steppermotor RAM limit %08x\n",
				//	(u32)ptr, (u32)limit);
				return (void  *)(-EINVAL);	// exceed table limit, return error
			}
			val = speedtable->pramp_table[i] | flag;
			fpga_writel(val, ptr);
			fpga_readl(&val1, ptr);//test
			ptr = (void *)((uint32_t)ptr+sizeof(uint32_t));
		}
	}
	return ptr;		// return next loading address
}


static __INLINE void  * fpga_ram_load_value(void  *addr, uint32_t value)
{
	fpga_writel(value, addr);
	addr = (void *)((uint32_t)addr + sizeof(uint32_t));
	return addr;		// return next loading address
}


static int32_t fpga_stepmotor_config(struct stepmotor_dev *motordev, const struct steppermotor_config *config)
{
	struct speed_info *speedinfo;
	int32_t ret, steps, speedlevel;
	uint32_t val;
	

	if (!motordev || !config)
		return -EINVAL;

	ret = steppermotor_check_config(motordev->pmotor, config);
	//if (IS_ERR_VALUE(ret))
	if(ret<0)
	{
		//dev_err(motor->dev, "error in configuration of steppermotor");
		return -ret;
	}

	if (steppermotor_is_running(motordev->pmotor->status))
		return -EBUSY;

	//mutex_lock(&stepmotor_mutex);

	// clear ramp table and speed profile table
	fpga_ram_clear((void *)((uint32_t)motordev->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP), FPGA_RAM_MOTOR_TABLE_RAMP_SIZE); 
	fpga_ram_clear((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_RAMP_ACCSIZE), FPGA_RAM_MOTOR_TABLE_COUNT_SIZE);

	if (config->steps_to_run == 1)	// single-step configuration
	{
		/* write speed table */
		val = SPEED_TO_COUNT(motordev->pmotor->feature.pullin_speed, motordev->stepping, motordev->clock_period);
		val |= FPGA_RAM_MOTOR_TABLE_RAMP_CONST1;
		ret = fpga_writel(val, (void *)((uint32_t)motordev->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP));

		/* write profile table */
		val = 1 * motordev->stepping;
		ret = fpga_writel(val, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_RAMP_ACCSIZE)); 
	}
	else	// multiple-steps configuration
	{
		struct motor_speedtable *speedtable, *stoptable_low, *stoptable_high;
		void  *addr, *ptr_ramp, *ptr_count;
		int32_t index, rsteps;
		int32_t speed1, speed2;

		speedinfo = config->speedinfo;
		steps = speedinfo->steps;
		speed1 = speedinfo->speed;

		// compose single-speed or multiple-speeds and load int RAM
		index = lookup_speedtable(motordev->pmotor, speed1);
		speedtable = motordev->prampinfo->speeds[index].accel_table;

		rsteps = steps * speedtable->stepping;

		ptr_ramp = (void *)((uint32_t)motordev->ram_base + FPGA_RAM_MOTOR_TABLE_RAMP);
		ptr_count = (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_RAMP_ACCSIZE);

		// load acceleration table of speed 1
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_RAMP_LIMIT, FPGA_RAM_MOTOR_TABLE_RAMP_ACCEL, speedtable); 
		//dev_dbg(motor->dev, "load acceleration ramp table (speed1) @ %08x, ret=%08x", (u32)ptr_ramp, (u32)addr);
		//if (IS_ERR(addr))
		if(addr<0)
			goto err_handling;
		else
			ptr_ramp = addr;


		// setup acceleration steps
		ptr_count = fpga_ram_load_value(ptr_count, speedtable->ramp_size);
		rsteps -= speedtable->ramp_size;

		speedtable = stoptable_low = stoptable_high = motordev->prampinfo->speeds[index].decel_table;

		// load constant-speed table entry of speed 1
		val = speedtable->pramp_table[0] | FPGA_RAM_MOTOR_TABLE_RAMP_CONST1; 
		ptr_ramp = fpga_ram_load_value(ptr_ramp, val);
		speedlevel = motordev->prampinfo->speeds[index].step_ticks; 

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
		addr = fpga_ram_load_ramptable(ptr_ramp, MOTOR_TABLE_RAMP_LIMIT, FPGA_RAM_MOTOR_TABLE_RAMP_DECEL|speedtable->dec_ramp_attr, speedtable);
		//dev_dbg(motor->dev, "load deceleration ramp table @ %08x, ret=%08x", (u32)ptr_ramp, (u32)addr);
err_handling:

	}

	/* setup motion direction */
	val = (config->dir == MOTION_CLOCKWISE) ? FPGA_REG_MOTOR_DIRECTION : 0;
	fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_DIRECTION, val);
#if 0
	/* setup preset running steps register */
	val = config->steps_to_run * motordev->stepping;
	fpga_writel(val, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_PRESET_STEPS));
#endif
	//mutex_unlock(&stepmotor_mutex);

	return 0;
}


static int32_t fpga_stepmotor_get_running_steps(struct stepmotor_dev *motordev)
{
	uint32_t val;
	int32_t rs;

	if (!motordev)
		return -EINVAL;

	rs = fpga_readl(&val, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_RUNNING_STEPS));
	if (rs)
		return rs;

	val = val / motordev->stepping;
	return val;
}

#if 0
static int32_t fpga_stepmotor_set_trigger_next(struct stepmotor_dev *motordev, const struct steppermotor_trigger *trigger)
{
	uint32_t val, val1;
	int32_t rs;

	if (!motordev)
		return -EINVAL;

	val = 0;
	if (trigger->control_set_trigger_stop) 
		val |= FPGA_REG_MOTOR_STOP_AT_TRGSTEP_END;
	if (trigger->control_set_trigger_sensor) 
		val |= FPGA_REG_MOTOR_SENSOR_CHECK_MODE;
	if (trigger->control_set_sensor_stop) 
		val |= FPGA_REG_MOTOR_SENSER_STOP_ENABLE;
	if (trigger->control_set_sensor_continue_mode) 	
		val |= FPGA_REG_MOTOR_SENSER_CONTINUE_MODE;
	if (trigger->control_set_sensor_stop_mode) 		
		val |= FPGA_REG_MOTOR_SENSER_STOP_MODE;

	fpga_readl(&val1, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL));	//test
	rs = fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), TRIGGER_NEXT_CTRL_MASK, val);
	fpga_readl(&val1, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL));	//test

	fpga_readl(&val1, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_TRIGGER_STEP_NEXT));//test
	val = (uint32_t)(trigger->steps * motordev->stepping);
	 fpga_writel(val, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_TRIGGER_STEP_NEXT));

	fpga_readl(&val1, (void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_TRIGGER_STEP_NEXT));//test
	 return 0;
}
#endif

irqreturn_t fpga_stepmotor_isr(int32_t irq, void *dev_id)
{
	struct stepmotor_dev *motordev;
	struct steppermotor *motor;
	int32_t status;

	if (!dev_id)
		return -EINVAL;

	motordev = (struct stepmotor_dev *)dev_id;
	motor = motordev->pmotor;

	status = STEPPERMOTOR_STOPED;
	fpga_update_lbits((void *)((uint32_t)motordev->mmio_base + FPGA_REG_MOTOR_CONTROL), FPGA_REG_MOTOR_RUN, 0);

	/* update steppermotor status */
	motor->status = status;

	if(motor->callback)
		motor->callback(motor, &motor->callbackdata);
	return 0;//IRQ_HANDLED;
}

struct steppermotor_ops fpga_stepmotor_ops = {
	fpga_stepmotor_config,
	fpga_stepmotor_status,
	fpga_stepmotor_start,
	fpga_stepmotor_stop,
	NULL,
	NULL,
	NULL,
	fpga_stepmotor_get_running_steps,
	NULL,
	NULL,
	NULL,
	NULL,
};


#endif // end of TODO
