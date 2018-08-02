/*
 * FPGA low-level IO driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */

#include <stdint.h>
#include "../fpga_io.h"
#include "../fpga.h"
#include "fpgadrv.h"


volatile uint32_t *ctrl_reg_base;
volatile uint32_t *ints_reg_base;


uint32_t fpga_get_version(void)
{
	uint32_t version;
	fpga_readl(&version, ctrl_reg_base + FPGA_REG_VERSION);
	return version;
}


void fpga_reset(void)
{
	uint32_t int_status;

	// disable all FPGA interrupts(enale global only)
	fpga_writel(FPGA_REG_INT_SEL_ALL, ints_reg_base + FPGA_REG_INT_ENABLE);
	// clear all pending FPGA interrupts
	fpga_readl(&int_status, ints_reg_base + FPGA_REG_INT_STATUS);
	fpga_writel(int_status, ints_reg_base + FPGA_REG_INT_CLEAR);
}


void fpga_enable_interrupt(uint32_t mask)
{
	fpga_update_lbits(ints_reg_base + FPGA_REG_INT_ENABLE, mask, mask);
}


void fpga_disable_interrupt(uint32_t mask)
{
	fpga_update_lbits(ints_reg_base + FPGA_REG_INT_ENABLE, mask, 0);
}


void fpga_clear_interrupt(uint32_t mask)
{
	fpga_writel(mask, ints_reg_base + FPGA_REG_INT_CLEAR);
}


int fpga_install(const struct fpga_resource *fpga_rc)
{
	if (!fpga_rc)
		return -1;

	ctrl_reg_base = (volatile uint32_t *)fpga_rc->ctrl_reg_base;
	ints_reg_base = (volatile uint32_t *)fpga_rc->ints_reg_base;

	fpga_reset();
	return 0;
}
