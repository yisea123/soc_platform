/*
 * FPGA-based simple GPIO driver
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

#include "fpga_gpio.h"

#define NB_OF_SGPIO	8

#define BITS_15_TO_8(v)	(((v) >> 8) & 0xff)
#define BITS_7_TO_0(v)	((v) & 0xff)


static void SGPIO_irqhandler(void *device, int num, void *data)
{
	sgpio_instance_t *gpio = (sgpio_instance_t *)data;
	uint32_t status, mask;
	int i;

	if (!gpio->irqmux)
		fpga_clear_interrupt(gpio->irqmask);

	/* get and clear interrupt flag of all ports */
	fpga_readl(&status, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_STATUS);
	if(!status)
		return;
	
	fpga_writel(status, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_CLEAR);
	fpga_writel(0, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_CLEAR);	
	
	mask = 1;
	for (i=0; i < NB_OF_SGPIO; i++)
	{
		if (status & mask && gpio->callback[i])
			gpio->callback[i](gpio, i, gpio->callbackdata[i]);
		mask <<= 1;
	}
}


/*-------------------------------------------------------------------------*//**
 * SGPIO_init()
 */
void SGPIO_init(sgpio_instance_t * gpio)
{
	int i;
	if (gpio->irqn == -1)
		return;

	/*  reset GPIO chip's interrupt control logics if available */
	fpga_writel(0u, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_ENABLE);
	fpga_writel(0xffu, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_CLEAR);
	fpga_writel(0, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_CLEAR);
	fpga_writel(0u, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_MODE_1);
	fpga_writel(0u, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_MODE_2);

	for (i=0; i < NB_OF_SGPIO; i++)
	{
		gpio->callback[i] = NULL;
		gpio->callbackdata[i] = NULL;
	}
	/* Configure IRQ handler */
	if (gpio->irqmux)
	{
		fpga_irqmux_set_irqhandler(gpio->irqmux, gpio->irqn, (irqcallback_t)SGPIO_irqhandler, gpio);
	}
	else 
	{
		fabric_irqcallback_install(gpio->irqn, (irqcallback_t)SGPIO_irqhandler, gpio);
		fpga_enable_interrupt(gpio->irqmask);
	}
}


/*-------------------------------------------------------------------------*//**
 * SGPIO_get_inputs
 */
uint32_t SGPIO_get_inputs(sgpio_instance_t * gpio)
{
	uint32_t gpio_in = 0;

	fpga_readl(&gpio_in, (char *)gpio->base_addr + FPGA_REG_GPIO_INPUT);

	return (gpio_in & 0xff);
}


/*-------------------------------------------------------------------------*//**
 * SGPIO_enable_irq
 */
void SGPIO_enable_irq(sgpio_instance_t * gpio, gpio_id_t port_id)
{
	uint32_t mask;

	HAL_ASSERT( port_id < NB_OF_SGPIO );
	HAL_ASSERT( gpio->irqn != -1 );

	mask = FPGA_REG_GPIO_MASK(port_id);
	fpga_update_lbits((char *)gpio->base_addr + FPGA_REG_GPIO_INT_ENABLE, mask, mask);
}


/*-------------------------------------------------------------------------*//**
 * SGPIO_disable_irq
 */
void SGPIO_disable_irq(sgpio_instance_t * gpio, gpio_id_t port_id)
{
	uint32_t mask;

	HAL_ASSERT( port_id < NB_OF_SGPIO );
	HAL_ASSERT( gpio->irqn != -1 );

	mask = FPGA_REG_GPIO_MASK(port_id);
	fpga_update_lbits((char *)gpio->base_addr + FPGA_REG_GPIO_INT_ENABLE, mask, 0);
}


/*-------------------------------------------------------------------------*//**
 * SGPIO_clear_irq
 */
void SGPIO_clear_irq(sgpio_instance_t * gpio, gpio_id_t port_id)
{
	uint32_t mask;

	HAL_ASSERT( port_id < NB_OF_SGPIO );
	HAL_ASSERT( gpio->irqn != -1 );

	mask = FPGA_REG_GPIO_MASK(port_id);
	fpga_writel(mask, (char *)gpio->base_addr + FPGA_REG_GPIO_INT_CLEAR);
}


/*-------------------------------------------------------------------------*//**
 * SGPIO_config_irq
 */
void SGPIO_config_irq(sgpio_instance_t * gpio, gpio_id_t port_id, uint32_t config)
{
	uint32_t mask;

	HAL_ASSERT( port_id < NB_OF_SGPIO );
	HAL_ASSERT( gpio->irqn != -1 );

	if (port_id < 4)
	{
		mask = FPGA_IRQ_MODE_MASK << (port_id * FPGA_IRQ_MODE_BITWIDTH);
		config = (config & FPGA_IRQ_MODE_MASK) << (port_id * FPGA_IRQ_MODE_BITWIDTH);
		fpga_update_lbits((char *)gpio->base_addr + FPGA_REG_GPIO_INT_MODE_1, mask, config);
	}
	else
	{
		port_id = (gpio_id_t)(port_id - 4);
		mask = FPGA_IRQ_MODE_MASK << (port_id * FPGA_IRQ_MODE_BITWIDTH);
		config = (config & FPGA_IRQ_MODE_MASK) << (port_id * FPGA_IRQ_MODE_BITWIDTH);
		fpga_update_lbits((char *)gpio->base_addr + FPGA_REG_GPIO_INT_MODE_2, mask, config);
	}
}


/*-------------------------------------------------------------------------*//**
 * SGPIO_set_irqcallback
 */
void SGPIO_set_irqcallback(sgpio_instance_t * gpio, gpio_id_t port_id, irqcallback_t callback, void *data)
{
	HAL_ASSERT( port_id < NB_OF_SGPIO );
	HAL_ASSERT( gpio->irqn != -1 );

	gpio->callback[port_id] = callback;
	gpio->callbackdata[port_id] = data;
}

