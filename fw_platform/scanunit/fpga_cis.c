/*
 * GWI FPGA-controlled CIS (Contact Image Sensor) driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */

#include "../fpga.h"
#include "../fpga_io.h"

#include "imagesensor.h"
#include "fpga_cis.h"


static int fpga_cis_enable(struct imagesensor *sensor)
{
	struct fpga_cis_resource *sensor_rc;
	int rs;
	uint32_t mask;

	if (!sensor)
		return -1;

	sensor_rc = (struct fpga_cis_resource *)sensor->resource;
	mask = FPGA_REG_CIS_SCAN_ENABLE; 
	rs = fpga_update_lbits((char *)sensor_rc->ctrl_base + FPGA_REG_CIS_CONTROL, mask, mask);
	return rs;
}


static void fpga_cis_disable(struct imagesensor *sensor)
{
	struct fpga_cis_resource *sensor_rc;
	uint32_t mask;

	if (!sensor)
		return;

	sensor_rc = (struct fpga_cis_resource *)sensor->resource;
	mask = FPGA_REG_CIS_SCAN_ENABLE;
	fpga_update_lbits((char *)sensor_rc->ctrl_base + FPGA_REG_CIS_CONTROL, mask, 0);
}


static int fpga_cis_get_config(struct imagesensor *sensor, struct scanunit_config *config)
{
	struct fpga_cis_resource *sensor_rc;
	struct scan_reg_config *regconfig;
	int i;

	if (!sensor || !config)
		return -1;
	if (config->regcount <= 0 || !config->regconfig) 
		return -1;

	sensor_rc = (struct fpga_cis_resource *)sensor->resource;
	regconfig = config->regconfig;
	for (i=0; i<config->regcount; i++) {
		uint32_t value, mask = regconfig[i].mask;
		fpga_readl(&value, (char *)sensor_rc->mmio_base + regconfig[i].address);
		if (mask != 0)
			value &= mask;
		regconfig[i].value = value;
	}
	return 0;
}


static int fpga_cis_set_config(struct imagesensor *sensor, const struct scanunit_config *config)
{
	struct fpga_cis_resource *sensor_rc;
	struct scan_reg_config *regconfig;
	int i;

	if (!sensor || !config)
		return -1;
	if (config->regcount <= 0 || !config->regconfig) 
		return -1;

	sensor_rc = (struct fpga_cis_resource *)sensor->resource;
	regconfig = config->regconfig;
	for (i=0; i<config->regcount; i++) {
		uint32_t value, mask = regconfig[i].mask;
		if (mask != 0) {	// bit operation
			fpga_readl(&value, (char *)sensor_rc->mmio_base + regconfig[i].address);
			value &= ~mask;
			value |= regconfig[i].value & mask;
		}
		else
			value = regconfig[i].value;
		fpga_writel(value, (char *)sensor_rc->mmio_base + regconfig[i].address);
	}
	return 0;
}


int fpga_cis_install(struct imagesensor *sensor)
{
	if (!sensor)
		return -1;
	if (!sensor->resource)
		return -1;

	sensor->enable = fpga_cis_enable;
	sensor->disable = fpga_cis_disable;
	sensor->get_config = fpga_cis_get_config;
	sensor->set_config = fpga_cis_set_config;
	return 0;
}


int fpga_cis_drvinit(void)
{
	return 0;
}