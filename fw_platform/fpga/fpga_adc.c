/*
 * FPGA-based Analog-to-Digital Converter driver
 *
 * Copyright 2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include <stdint.h>
#include "hal_assert.h"

#include "fpga.h"
#include "fpga_io.h"
#include "fpgadrv.h"
#include "irqcallback.h"
#include "adc.h"
#include "fpga_adc.h"



#define BITS_23_TO_16(v)	(((v) >> 16) & 0xff)
#define BITS_15_TO_8(v)		(((v) >> 8) & 0xff)
#define BITS_11_TO_4(v)		(((v) >> 4) & 0xff)
#define BITS_7_TO_0(v)		((v) & 0xff)


static void fpga_adc_irqhandler(void *device, int num, void *data)
{
	struct ad_converter *adc = (struct ad_converter *)data;
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;
	uint32_t status, value, mask;
	int i;

	fpga_clear_interrupt(adc_rc->irqmask);

	/* get and clear interrupt flag of all channels */
	fpga_readl(&value, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_STATUS_1);
	fpga_writel(value, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_CLEAR_1);
	status = value & 0xff;
	fpga_readl(&value, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_STATUS_2);
	fpga_writel(value, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_CLEAR_2);
	status |= (value & 0xff) << 8;
	fpga_readl(&value, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_STATUS_3);
	fpga_writel(value, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_CLEAR_3);
	status |= (value & 0xff) << 16;

	mask = 1;
	for (i=0; i < adc->channels; i++)
	{
		if (status & mask && adc->callback[i])
			adc->callback[i](adc, i, adc->callbackdata[i]);
		mask <<= 1;
	}
}


static int fpga_adc_init(struct ad_converter *adc, void *data)
{
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;

	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_ENABLE_1);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_ENABLE_2);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_ENABLE_3);
	fpga_writel(0xffu, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_CLEAR_1);
	fpga_writel(0xffu, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_CLEAR_2);
	fpga_writel(0xffu, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_CLEAR_3);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_1);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_2);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_3);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_4);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_5);
	fpga_writel(0u, (char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_6);

	return 0;
}


static void fpga_adc_enable(struct ad_converter *adc, int channel)
{
}


static void fpga_adc_disable(struct ad_converter *adc, int channel)
{
}


static int fpga_adc_status(struct ad_converter *adc, uint32_t *status)
{
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;
	uint32_t val1, val2, val3 = 0;

	fpga_readl(&val1, (char *)adc_rc->base_addr + FPGA_REG_ADC_COMPARE_1);
	fpga_readl(&val2, (char *)adc_rc->base_addr + FPGA_REG_ADC_COMPARE_2);
	fpga_readl(&val3, (char *)adc_rc->base_addr + FPGA_REG_ADC_COMPARE_3);
	*status = (val3 << 16) + (val2 << 8) + val1;

	return 0;
}


static int fpga_adc_read_average(struct ad_converter *adc, int channel, uint32_t *value)
{
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;
	uint32_t data;

	fpga_readl(&data, (char *)adc_rc->base_addr + FPGA_REG_ADC_AVERAGE_1 + channel * adc_rc->bus_width);
	*value = (data & 0xff) << 4;	// register value to ADC input
	return 0;
}


static int fpga_adc_read_compare(struct ad_converter *adc, int channel, uint32_t *value)
{
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;
	uint32_t data, mask;
	
	fpga_readl(&data, (char *)adc_rc->base_addr + FPGA_REG_ADC_COMPARE_1 + (channel / 8) * adc_rc->bus_width);
	mask = FPGA_REG_ADC_MASK(channel % 8);
	*value = (data & mask) ? 1 : 0;

	return 0;
}


static int fpga_adc_set_threshold(struct ad_converter *adc, int channel, uint32_t value)
{
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;
	uint32_t val;

	val = BITS_11_TO_4(value);	// use only higher bit[11:4] and ignore lower bit[3:0]
	fpga_writel(val, (char *)adc_rc->base_addr + FPGA_REG_ADC_THRESHOLD_1 + channel * adc_rc->bus_width);

	return 0;
}


static int fpga_adc_set_event(struct ad_converter *adc, int channel, adc_event_t event)
{
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;
	uint32_t config, mask;

	/* set interrupt mode of this ADC channel */
	switch (event) {
	case ADC_EVT_NONE:
		config = FPGA_IRQ_MODE_NONE;
		break;
	case ADC_EVT_CHANGE_RISING:
		config = FPGA_IRQ_MODE_RISING_EDGE;
		break;
	case ADC_EVT_CHANGE_FALLING:
		config = FPGA_IRQ_MODE_FALLING_EDGE;
		break;
	case ADC_EVT_CHANGE_EITHER:
		config = FPGA_IRQ_MODE_BOTH_EDGES;
		break;
	}
	mask = FPGA_IRQ_MODE_MASK << ((channel % 4) * FPGA_IRQ_MODE_BITWIDTH);
	config = (config & FPGA_IRQ_MODE_MASK) << ((channel % 4) * FPGA_IRQ_MODE_BITWIDTH);
	fpga_update_lbits((char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_1 + (channel / 4) * adc_rc->bus_width, mask, config);

	/* enable interrupt of this ADC channel */
	mask = FPGA_REG_ADC_MASK(channel % 8);
	fpga_update_lbits((char *)adc_rc->base_addr + FPGA_REG_ADC_INT_ENABLE_1 + (channel / 8) * adc_rc->bus_width, mask, mask);

	return 0;
}


static int fpga_adc_unset_event(struct ad_converter *adc, int channel)
{
	const struct fpga_adc_resource *adc_rc = (const struct fpga_adc_resource *)adc->resource;
	uint32_t config, mask;

	config = FPGA_IRQ_MODE_NONE;
	mask = FPGA_IRQ_MODE_MASK << ((channel % 4) * FPGA_IRQ_MODE_BITWIDTH);
	config = (config & FPGA_IRQ_MODE_MASK) << ((channel % 4) * FPGA_IRQ_MODE_BITWIDTH);
	fpga_update_lbits((char *)adc_rc->base_addr + FPGA_REG_ADC_INT_MODE_1 + (channel / 4) * adc_rc->bus_width, mask, config);

	/* disable interrupt of this ADC channel */
	mask = FPGA_REG_ADC_MASK(channel % 8);
	fpga_update_lbits((char *)adc_rc->base_addr + FPGA_REG_ADC_INT_ENABLE_1 + (channel / 8) * adc_rc->bus_width, mask, 0);

	return 0;
}


int fpga_adc_install(struct ad_converter *adc)
{
	const struct fpga_adc_resource *adc_rc;

	if (!adc)
		return -1;
	if (!adc->resource)
		return -1;

	/* setup ADC device operation functions */
	adc->init = fpga_adc_init;
	adc->enable = fpga_adc_enable;
	adc->disable = fpga_adc_disable;
	adc->status = fpga_adc_status;
	adc->read_raw = adc->read_average = fpga_adc_read_average;
	adc->read_compare = fpga_adc_read_compare;
	adc->set_threshold = fpga_adc_set_threshold;
	adc->set_event = fpga_adc_set_event;
	adc->unset_event = fpga_adc_unset_event;

	adc_rc = (const struct fpga_adc_resource *)adc->resource;

	/* Configure IRQ handler */
	fabric_irqcallback_install(adc_rc->irqn, (irqcallback_t)fpga_adc_irqhandler, adc);
	fpga_enable_interrupt(adc_rc->irqmask);

	return 0;
}


int fpga_adc_drvinit(void)
{
	return 0;
}
