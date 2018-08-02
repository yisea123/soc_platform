/*
 * Image Scan Unit driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */

#include "irqcallback.h"
#include "fpga_io.h"
#include "fpga.h"
#include "fpgadrv.h"
#include "scanunit.h"
#include "scandrv.h"
#include "imagedigitiser.h"
#include "imagesensor.h"


static volatile uint32_t *scanctrl_reg_base;
static uint32_t scan_int_mask;


static void scanunit_irqcallback(void *device, int id, void *data)
{
	fpga_clear_interrupt(scan_int_mask);
}


void scanunit_reset(void)
{
	fpga_writel(0, scanctrl_reg_base + FPGA_REG_CIS_CONTROL);
	fpga_clear_interrupt(scan_int_mask);
}


void scanunit_set_timeout_value(unsigned int timeout)
{
}


void scanunit_start_scanning(void)
{
	fpga_clear_interrupt(scan_int_mask);
	fpga_enable_interrupt(scan_int_mask);
	fpga_update_lbits(scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_SCAN_ENABLE, FPGA_REG_CIS_SCAN_ENABLE);
}


void scanunit_stop_scanning(void)
{
	fpga_disable_interrupt(scan_int_mask);
	fpga_update_lbits(scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_SCAN_ENABLE, 0);
}


void scanunit_turnon_lights(void)
{
	fpga_update_lbits(scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_LEDS_ENABLE, FPGA_REG_CIS_LEDS_ENABLE);
}


void scanunit_turnoff_lights(void)
{
	fpga_update_lbits(scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_LEDS_ENABLE, 0);
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


int scanunit_install(const struct scanunit_resource *scanunit_rc)
{
	if (!scanunit_rc)
		return -1;

	scanctrl_reg_base = (volatile uint32_t *)(scanunit_rc->ctrl_base);
	scan_int_mask = scanunit_rc->int_mask;

	scanunit_reset();
	fabric_irqcallback_install(scanunit_rc->fabric_irq, (irqcallback_t)scanunit_irqcallback, NULL);

	return 0;
}
