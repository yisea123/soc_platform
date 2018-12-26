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


extern const struct scanunit_hwinfo scanner_hwinfo;
extern struct imagedigitiser imagedigitiser_list[];
extern struct imagesensor imagesensor_list[];

static volatile uint32_t *scanctrl_reg_base;
static uint32_t scan_int_mask;


static void scanunit_irqcallback(void *device, int id, void *data)
{
	fpga_clear_interrupt(scan_int_mask);
}


void scanunit_reset(void)
{
	fpga_writel(0, (char *)scanctrl_reg_base + FPGA_REG_CIS_CONTROL);
	fpga_clear_interrupt(scan_int_mask);
}


void scanunit_set_timeout_value(unsigned int timeout)
{
	fpga_writel(timeout, (char *)scanctrl_reg_base + FPGA_REG_CIS_MAX_LIGHTON_TIME);
}


void scanunit_start_scanning(void)
{
	imagedigitiser_enable(imagedigitiser_get(0));
	fpga_clear_interrupt(scan_int_mask);
	fpga_enable_interrupt(scan_int_mask);
	fpga_update_lbits((char *)scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_SCAN_ENABLE, FPGA_REG_CIS_SCAN_ENABLE);
}


void scanunit_stop_scanning(void)
{
	fpga_disable_interrupt(scan_int_mask);
	fpga_update_lbits((char *)scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_SCAN_ENABLE, 0);
	imagedigitiser_disable(imagedigitiser_get(0));
}


void scanunit_turnon_lights(void)
{
	fpga_update_lbits((char *)scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_LEDS_ENABLE, FPGA_REG_CIS_LEDS_ENABLE);
}


void scanunit_turnoff_lights(void)
{
	fpga_update_lbits((char *)scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_LEDS_ENABLE, 0);
}


void scanunit_set_scanning_mode(struct scanunit_scanmode mode)
{
	fpga_update_lbits((char *)scanctrl_reg_base + FPGA_REG_CIS_CONTROL, FPGA_REG_CIS_SCANMODE_MASK, mode.ledmode);
	fpga_writel(mode.dpimode, (char *)scanctrl_reg_base + FPGA_REG_CIS_DPI);
}


int scanunit_get_scanlines(void)
{
	uint32_t val;
	fpga_readl(&val, (char *)scanctrl_reg_base + FPGA_REG_CIS_SCANLINES);
	return (int)val;
}

int scanunit_get_wr_addr(void)
{
	uint32_t val=0, tmp1=0, tmp2=0;
	fpga_readl(&tmp1, (char *)scanctrl_reg_base + FPGA_REG_CIS_DDR_PRESENT_WR_ADDR_L);
	fpga_readl(&tmp2, (char *)scanctrl_reg_base + FPGA_REG_CIS_DDR_PRESENT_WR_ADDR_H);
	val = (tmp2<<16) + tmp1;
	return (int)val;
}

int scanunit_get_hwinfo(const struct scanunit_hwinfo *hwinfo)
{
	if (!hwinfo)
		return -1;
	hwinfo = &scanner_hwinfo;
	return 0;
}


int scanunit_get_digitiser_config(int device, struct scanunit_config *config)
{
	int rs;

	if (device < 0 || device >= scanner_hwinfo.digitisers) 
		return -1;
	if (config->regcount <= 0 || config->regconfig == NULL)
		return -1;

	rs = imagedigitiser_get_config(&imagedigitiser_list[device], config);

	return rs;
}


int scanunit_set_digitiser_config(int device, const struct scanunit_config *config)
{
	int rs;

	if (device < 0 || device >= scanner_hwinfo.digitisers) 
		return -1;
	if (config->regcount <= 0 || config->regconfig == NULL)
		return -1;

	rs = imagedigitiser_set_config(&imagedigitiser_list[device], config);

	return rs;
}


int scanunit_get_digitiser_aux_config(int device, struct scanunit_config *config)
{
	int rs;

	if (device < 0 || device >= scanner_hwinfo.digitisers) 
		return -1;
	if (config->regcount <= 0 || config->regconfig == NULL)
		return -1;

	rs = imagedigitiser_get_aux_config(&imagedigitiser_list[device], config);

	return rs;
}


int scanunit_set_digitiser_aux_config(int device, const struct scanunit_config *config)
{
	int rs;

	if (device < 0 || device >= scanner_hwinfo.digitisers)
		return -1;
	if (config->regcount <= 0 || config->regconfig == NULL)
		return -1;

	rs = imagedigitiser_set_aux_config(&imagedigitiser_list[device], config);

	return rs;
}


int scanunit_get_sensor_config(int device, struct scanunit_config *config)
{
	int rs;

	if (device < 0 || device >= scanner_hwinfo.sensors)
		return -1;
	if (config->regcount <= 0 || config->regconfig == NULL)
		return -1;

	rs = imagesensor_get_config(&imagesensor_list[device], config);

	return rs;
}


int scanunit_set_sensor_config(int device, const struct scanunit_config *config)
{
	int rs;

	if (device < 0 || device >= scanner_hwinfo.sensors)
		return -1;
	if (config->regcount <= 0 || config->regconfig == NULL)
		return -1;

	rs = imagesensor_set_config(&imagesensor_list[device], config);

	return rs;
}


int scanunit_get_sensor_common_config(struct scan_reg_config *regconfig)
{
	uint32_t value;

	if (!regconfig)
		return -1;

	fpga_readl(&value, (char *)scanctrl_reg_base + regconfig->address);
	regconfig->value = value & regconfig->mask;

	return 0;
}


int scanunit_set_sensor_common_config(const struct scan_reg_config *regconfig)
{
	uint32_t value;

	if (!regconfig)
		return -1;

	if (regconfig->mask != 0) {     // bit operation
		fpga_readl(&value, (char *)scanctrl_reg_base + regconfig->address);
		value &= ~regconfig->mask;
		value |= regconfig->value & regconfig->mask;
	}
	else
		value = regconfig->value;
	fpga_writel(value, (char *)scanctrl_reg_base + regconfig->address);

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
