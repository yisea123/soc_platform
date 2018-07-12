/*
 * generic DC-motor driver functions implementation
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "dcmotor.h"


int dcmotor_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<dcmotor_num; i++)
	{
		if (dcmotor_list[i].install != NULL)
		{
			rs = dcmotor_list[i].install(&dcmotor_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct dcmotor *dcmotor_get(int index)
{
	if (index >= dcmotor_num || index < 0)
		return NULL;
	else
		return &dcmotor_list[index];
}


int dcmotor_set_callback(struct dcmotor *motor, void (*callback)(struct dcmotor *, struct callback_data *),
			struct callback_data *data)
{
	if (!motor)
		return -1;
	motor->callback = callback;
	motor->callbackdata = *data;
	return 0;
}


int dcmotor_start(struct dcmotor *motor)
{
	if (motor)
		return motor->ops->start(motor);

	return -1;
}


void dcmotor_stop(struct dcmotor *motor)
{
	if (motor)
		motor->ops->stop(motor);
}


int dcmotor_status(struct dcmotor *motor)
{
	if (!motor)
		return -1;
	return motor->status;
}


int dcmotor_get_config(struct dcmotor *motor, struct dcmotor_config *config)
{
	if (!motor || !config)
		return -1;
	*config = motor->config;
	return 0;
}


int dcmotor_set_config(struct dcmotor *motor, const struct dcmotor_config *config)
{
	if (!motor || !config)
		return -1;
	motor->config = *config;
	return motor->ops->config(motor, config);
}