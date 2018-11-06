/*
 * generic steppermotor driver functions implementation
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "steppermotor.h"
#include "steptable.h"


static inline void time_to_count(int count, uint32_t *ramp_table, int clock)
{
	int i;
	for (i = 0; i < count; i++)
		ramp_table[i] /= clock;
}

/*
 * motor_ramptable_convert() - convert all time values from nanoseconds to ticks in a steppermotor's ramptable
 * @dev: device structure pointer of the steppermotor
 *
 * convert all time value to ticks in ramptable of a steppermotor
 */
int steppermotor_ramptable_convert(struct ramp_info *rampinfo, int clock)
{
	int i;

	if (!rampinfo || clock<=0)
		return -1;

	for (i = 0; i < rampinfo->num_speed; i++)
	{
		struct motor_speedtable *speedtable;

		speedtable = rampinfo->speeds[i].accel_table;
		time_to_count(speedtable->ramp_size, speedtable->ramp_table, clock);
		speedtable = rampinfo->speeds[i].decel_table;
		
		if(speedtable->dec_ramp_attr != DEC_RAMP_REVERSED)
		{			
			time_to_count(speedtable->ramp_size, speedtable->ramp_table, clock);
			rampinfo->speeds[i].step_ticks = speedtable->ramp_table[0];
		}
		else
			rampinfo->speeds[i].step_ticks = speedtable->ramp_table[speedtable->ramp_size-1];
		
	}
	return 0;
}


int steppermotor_check_config(struct steppermotor *motor, const struct steppermotor_config *config)
{
	struct speed_info *speedinfo;
	int i, ret, steps, index1, speed1;

	/* check total steps and number of speed configuration */
	if (config->steps_to_run <= 0 || config->steps_to_run > motor->feature.max_steps || config->num_speed <= 0) 
	{
//		dev_err("steppermotor_check_config error:steps %d %d or num_speed %d\n", config->steps_to_run, motor->feature.max_steps, config->num_speed);
		return -1;
	}

	/* check speed configuration */
	steps = 0;
	speedinfo = config->speedinfo;
	for (i=0; i<config->num_speed; i++)
	{
		if (!speedinfo)
		{
//			dev_err("steppermotor_check_config error:speedinfo\n");
			return -1;
		}

		ret = lookup_speedtable(motor, speedinfo->speed);
		if (ret == -1)	// requested speed is not supported
		{
//			dev_err("steppermotor_check_config error:speed %d\n", speedinfo->speed);
			return -1;
		}

		steps += speedinfo->steps;
		speedinfo = speedinfo->nextspeed;
	}
	if (steps != config->steps_to_run)
	{
//		dev_err("steppermotor_check_config error:steps_to_run %d %d\n", steps, config->steps_to_run);
		return -1;
	}

	if (config->steps_to_run == 1)
		return 0;


	/* check with acceleration/deceleration steps */
	steps = config->steps_to_run;
	speedinfo = config->speedinfo;
	speed1 = speedinfo->speed;
	index1 = lookup_speedtable(motor, speed1);
	if (index1 < 0)
		return -1;

	steps -= motor->feature.speeds[index1].accel_steps;
	if (steps <= 0)
		return -1;

	steps -= motor->feature.speeds[index1].decel_steps;
	if (steps <= 0)
		return -1;
	return 0;
}


int steppermotor_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<steppermotor_num; i++)
	{
		if (steppermotor_list[i].install != NULL)
		{
			rs = steppermotor_list[i].install(&steppermotor_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct steppermotor *steppermotor_get(int index)
{
	if (index >= steppermotor_num || index < 0)
		return NULL;
	else
		return &steppermotor_list[index];
}


int steppermotor_set_callback(struct steppermotor *motor, void (*callback)(struct steppermotor *, struct callback_data *),
			      struct callback_data *data)
{
	if (!motor || !callback)
		return -1;
	motor->callback = callback;
	motor->callbackdata = *data;
	return 0;
}


int steppermotor_start(struct steppermotor *motor)
{
	if (motor)
		return motor->ops->start(motor);

	return -1;
}


void steppermotor_stop(struct steppermotor *motor)
{
	if (motor)
		motor->ops->stop(motor);
}


void steppermotor_emergencybrake(struct steppermotor *motor)
{
	if (motor)
		motor->ops->emergencybrake(motor);
}


void steppermotor_lock(struct steppermotor *motor)
{
	if (motor && motor->ops->lock)
		motor->ops->lock(motor);
}


void steppermotor_unlock(struct steppermotor *motor)
{
	if (motor && motor->ops->unlock)
		motor->ops->unlock(motor);
}


int steppermotor_status(struct steppermotor *motor)
{
	if (!motor)
		return -1;
	return motor->status;
}


int steppermotor_get_running_steps(struct steppermotor *motor)
{
	if (motor && motor->ops->get_running_steps)
		return motor->ops->get_running_steps(motor);

	return -1;
}

int steppermotor_set_running_steps(struct steppermotor *motor, int steps)
{
	if (motor && motor->ops->set_running_steps)
		return motor->ops->set_running_steps(motor, steps);

	return -1;
}

int steppermotor_get_config(struct steppermotor *motor, struct steppermotor_config *config)
{
	if (!motor || !config)
		return -1;
	*config = motor->config;
	return 0;
}


int steppermotor_set_config(struct steppermotor *motor, const struct steppermotor_config *config)
{
	if (!motor || !config)
		return -1;
	motor->config = *config;
	return motor->ops->config(motor, config);
}
