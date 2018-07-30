/*
 * Analog and Digital Photo Sensor driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "photosensor.h"
#include "../fpga_io.h"
#include "../fpga.h"



int photosensor_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<photosensor_num; i++)
	{
		if (photosensor_list[i].install != NULL)
		{
			rs = photosensor_list[i].install(&photosensor_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct photosensor *photosensor_get(int index)
{
	if (index >= photosensor_num || index < 0)
		return NULL;
	else
		return &photosensor_list[index];
}


int photosensor_enable(struct photosensor *sensor)
{
	if (sensor)
		return sensor->ops->enable(sensor);

	return -1;
}


int photosensor_disable(struct photosensor *sensor)
{
	if (sensor)
		return sensor->ops->disable(sensor);

	return -1;
}


int photosensor_status(struct photosensor *sensor, int *status)
{
	if (sensor)
		return sensor->ops->status(sensor, status);

	return -1;
}


/**
 * photosensor_read_input() - read raw input value from a photosensor
 * @sensor: photosensor
 */
int photosensor_read_input(struct photosensor *sensor, uint32_t *val)
{
	if (sensor)
		return sensor->ops->read_input(sensor, val);

	return -1;
}


int photosensor_get_feature(struct photosensor *sensor, struct photosensor_feature *feature)
{
	if (!sensor || !feature)
		return -1;
	*feature = sensor->feature;
	return 0;
}


int photosensor_get_config(struct photosensor *sensor, struct photosensor_config *config)
{
	if (!sensor || !config)
		return -1;
	*config = sensor->config;
	return 0;
}


int photosensor_set_config(struct photosensor *sensor, const struct photosensor_config *config)
{
	if (!sensor || !config)
		return -1;
	sensor->config = *config;
	return sensor->ops->config(sensor, config);
}


int photosensor_set_event(struct photosensor *sensor, sensor_event_t event, void (*eventhandle)(struct photosensor *, sensor_event_t, void *), void *data)
{
	if (!sensor)
		return -1;
	sensor->eventtrigger = event;
	sensor->eventhandle = eventhandle;
	sensor->handledata = data;
	return sensor->ops->set_event(sensor, event, eventhandle, data);
}


int photosensor_unset_event(struct photosensor *sensor, sensor_event_t event)
{
	if (!sensor)
		return -1;
	return sensor->ops->unset_event(sensor, event);
}
