/*
 * general image sensor functions implementation
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "imagesensor.h"


int imagesensor_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<imagesensor_num; i++)
	{
		if (imagesensor_list[i].install != NULL)
		{
			rs = imagesensor_list[i].install(&imagesensor_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct imagesensor *imagesensor_get(int index)
{
	if (index >= imagesensor_num || index < 0)
		return NULL;
	else
		return &imagesensor_list[index];
}


int imagesensor_enable(struct imagesensor *sensor)
{
	if (sensor)
		return sensor->enable(sensor);

	return -1;
}


void imagesensor_disable(struct imagesensor *sensor)
{
	if (sensor)
		sensor->disable(sensor);
}


int imagesensor_get_config(struct imagesensor *sensor, struct scanunit_config *config)
{
	if (sensor)
		return sensor->get_config(sensor, config);

	return -1;
}


int imagesensor_set_config(struct imagesensor *sensor, const struct scanunit_config *config)
{
	if (sensor)
		return sensor->set_config(sensor, config);

	return -1;
}