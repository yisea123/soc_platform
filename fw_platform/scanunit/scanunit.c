/*
 * Image Scan Unit driver
 *
 * Copyright 2016 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 * Licensed under the GPL-2.
 */

#include "../fpga_io.h"
#include "../fpga.h"
#include "scanunit.h"
#include "imagedigitiser.h"
#include "imagesensor.h"



void scanunit_initialize()
{
}


static int scanunit_isr(int irq, void *dev_id)
{
	return 0;
}


void scanunit_reset(void)
{
}


void scanunit_set_timeout_value(unsigned int timeout)
{
}


void scanunit_start_scanning(void)
{
}


void scanunit_stop_scanning(void)
{
}


void scanunit_turnon_lights(void)
{
}


void scanunit_turnoff_lights(void)
{
}


void scanunit_set_scanning_mode(int mode)
{
}


int scanunit_get_hwinfo(struct scanunit_hwinfo *hwinfo)
{
	return 0;
}


int scanunit_get_digitiser_config(int device, struct scanunit_config *config)
{
	return 0;
}


int scanunit_set_digitiser_config(int device, const struct scanunit_config *config)
{
	return 0;
}


int scanunit_get_sensor_config(int device, struct scanunit_config *config)
{
	return 0;
}


int scanunit_set_sensor_config(int device, const struct scanunit_config *config)
{
	return 0;
}


int scanunit_get_sensor_common_config(struct scan_reg_config *regconfig)
{
	return 0;
}


int scanunit_set_sensor_common_config(const struct scan_reg_config *regconfig)
{
	return 0;
}