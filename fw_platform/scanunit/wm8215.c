/*
 * GWI FPGA-controlled Image Analog Front End (AFE) driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "sys_config.h"
#include "mss_spi.h"

#include "fpga.h"
#include "fpga_io.h"

#include "imagedigitiser.h"
#include "wm8215.h"


static uint8_t spi_wm8215_read_register(mss_spi_instance_t *mss_spi, uint8_t address)
{
        uint8_t data = 0;
        uint32_t frame;

        frame = (address | 0x10) << 8;

        MSS_SPI_set_slave_select(mss_spi, MSS_SPI_SLAVE_0);

        /* transfer command frame */
        MSS_SPI_transfer_frame(mss_spi, frame);

        /* read response data */
        MSS_SPI_transfer_block(mss_spi, &data, 0, &data, 1);

        MSS_SPI_clear_slave_select(mss_spi, MSS_SPI_SLAVE_0);

        return data;
}


static void spi_wm8215_write_register(mss_spi_instance_t *mss_spi, uint8_t address, uint8_t value)
{
        uint32_t frame;

        frame = ((address & 0x2f) << 8) + value;

        /* transfer command frame */
        MSS_SPI_set_slave_select(mss_spi, MSS_SPI_SLAVE_0);
        MSS_SPI_transfer_frame(mss_spi, frame);
        MSS_SPI_clear_slave_select(mss_spi, MSS_SPI_SLAVE_0);
}


static int wm8215_enable(struct imagedigitiser *afe)
{
	struct wm8215_resource *afe_rc;
	int rs;
	uint32_t mask;

	if (!afe)
		return -1;

	afe_rc = (struct wm8215_resource *)afe->resource;
	mask = afe_rc->mask;
	rs = fpga_update_lbits((char *)afe_rc->ctrl_base + FPGA_REG_IMGADC_CONTROL, mask, mask);
	return rs;
}


static void wm8215_disable(struct imagedigitiser *afe)
{
	struct wm8215_resource *afe_rc;
	uint32_t mask;

	if (!afe)
		return;

	afe_rc  = (struct wm8215_resource *)afe->resource;
	mask = afe_rc->mask;
	fpga_update_lbits((char *)afe_rc->ctrl_base + FPGA_REG_IMGADC_CONTROL, mask, 0);
}


static int wm8215_get_config(struct imagedigitiser *afe, struct scanunit_config *config)
{
	struct wm8215_resource *afe_rc;
	struct scan_reg_config *regconfig;
	int i;

	if (!afe || !config)
		return -1;
	if (config->regcount <= 0 || !config->regconfig) 
		return -1;

	afe_rc = (struct wm8215_resource *)afe->resource;
	regconfig = config->regconfig;
	for (i=0; i<config->regcount; i++) {
		uint32_t value, mask = regconfig[i].mask;
		value = spi_wm8215_read_register(afe_rc->mss_spi, regconfig[i].address);
		if (mask != 0)
			value &= mask;
		regconfig[i].value = value;
	}
	return 0;
}


static int wm8215_set_config(struct imagedigitiser *afe, const struct scanunit_config *config)
{
	struct wm8215_resource *afe_rc;
	struct scan_reg_config *regconfig;
	int i;

	if (!afe || !config)
		return -1;
	if (config->regcount <= 0 || !config->regconfig) 
		return -1;

	afe_rc = (struct wm8215_resource *)afe->resource;
	regconfig = config->regconfig;
	for (i=0; i<config->regcount; i++) {
		uint32_t value, mask = regconfig[i].mask;
		value = regconfig[i].value & mask;
		spi_wm8215_write_register(afe_rc->mss_spi, regconfig[i].address, (uint8_t)value);
	}
	return 0;
}


static int wm8215_get_aux_config(struct imagedigitiser *afe, struct scanunit_config *config)
{
	struct wm8215_resource *afe_rc;
	struct scan_reg_config *regconfig;
	int i;

	if (!afe || !config)
		return -1;
	if (config->regcount <= 0 || !config->regconfig) 
		return -1;

	afe_rc = (struct wm8215_resource *)afe->resource;
	regconfig = config->regconfig;
	for (i=0; i<config->regcount; i++) {
		uint32_t value, mask = regconfig[i].mask;
		fpga_readl(&value, (char *)afe_rc->mmio_base + regconfig[i].address);
		if (mask != 0)
			value &= mask;
		regconfig[i].value = value;
	}
	return 0;
}


static int wm8215_set_aux_config(struct imagedigitiser *afe, const struct scanunit_config *config)
{
	struct wm8215_resource *afe_rc;
	struct scan_reg_config *regconfig;
	int i;

	if (!afe || !config)
		return -1;
	if (config->regcount <= 0 || !config->regconfig) 
		return -1;

	afe_rc = (struct wm8215_resource *)afe->resource;
	regconfig = config->regconfig;
	for (i=0; i<config->regcount; i++) {
		fpga_update_lbits((char *)afe_rc->mmio_base + regconfig[i].address, regconfig[i].mask, regconfig[i].value);
	}
	return 0;
}


int wm8215_install(struct imagedigitiser *afe)
{
	struct wm8215_resource *afe_rc;
	uint32_t clk_div;
	if (!afe)
		return -1;
	if (!afe->resource)
		return -1;

	afe->enable = wm8215_enable;
	afe->disable = wm8215_disable;
	afe->get_config = wm8215_get_config;
	afe->set_config = wm8215_set_config;
	afe->get_aux_config = wm8215_get_aux_config;
	afe->set_aux_config = wm8215_set_aux_config;

	afe_rc = (struct wm8215_resource *)afe->resource;
	/*
	*  Initialize and Configure SPIx as Master.
	*/
	MSS_SPI_init(afe_rc->mss_spi);
	if (afe_rc->spi_clk_freq != 0)
		clk_div = MSS_SYS_APB_1_CLK_FREQ/afe_rc->spi_clk_freq;
	else
		clk_div = 2;
	MSS_SPI_configure_master_mode(afe_rc->mss_spi, MSS_SPI_SLAVE_0, MSS_SPI_MODE0, clk_div, 6+8);

	MSS_SPI_set_slave_select(afe_rc->mss_spi, MSS_SPI_SLAVE_0);

	return 0;
}


int wm8215_drvinit(void)
{
	return 0;
}