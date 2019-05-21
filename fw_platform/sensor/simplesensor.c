/*
 * Simple Sensor driver (device-independant abstraction layer)
 *
 * Copyright 2018,2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "simplesensor.h"


int simplesensor_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<simplesensor_num; i++)
	{
		if (simplesensor_list[i]->install != NULL)
		{
			rs = simplesensor_list[i]->install(simplesensor_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct simplesensor *simplesensor_get(int index)
{
	if (index >= simplesensor_num || index < 0)
		return NULL;
	else
		return simplesensor_list[index];
}


int simplesensor_enable(struct simplesensor *sensor)
{
	if (sensor)
		return sensor->ops->enable(sensor);

	return -1;
}


int simplesensor_disable(struct simplesensor *sensor)
{
	if (sensor)
		return sensor->ops->disable(sensor);

	return -1;
}


int simplesensor_status(struct simplesensor *sensor, int *status)
{
	if (sensor)
		return sensor->ops->status(sensor, status);

	return -1;
}


int simplesensor_read_input(struct simplesensor *sensor, uint32_t *val)
{
	if (sensor)
		return sensor->ops->read_input(sensor, val);

	return -1;
}


int simplesensor_set_event(struct simplesensor *sensor, sensor_event_t event, void (*eventhandler)(struct simplesensor *, sensor_event_t, void *), void *data)
{
	if (!sensor)
		return -1;
	sensor->eventtrigger = event;
	sensor->eventhandler = eventhandler;
	sensor->handlerdata = data;
	return sensor->ops->set_event(sensor, event, eventhandler, data);
}


int simplesensor_unset_event(struct simplesensor *sensor, sensor_event_t event)
{
	if (!sensor)
		return -1;
	return sensor->ops->unset_event(sensor, event);
}
