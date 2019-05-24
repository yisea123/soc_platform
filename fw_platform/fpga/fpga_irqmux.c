/*
 * FPGA-based IRQ mux driver
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

#include "fpga_irqmux.h"


static void fpga_irqmux_irqhandler(void *device, int num, void *data)
{
	fpga_irqmux_t *mux = (fpga_irqmux_t *)data;
	uint32_t status, mask;
	int i;

	fpga_clear_interrupt(mux->irqmask);
	status = mux->status;

	mask = 1;
	for (i=0; i < mux->mux_irqs; i++)
	{
		if (status & mask && mux->callback[i])
			mux->callback[i](mux, i, mux->callbackdata[i]);
		mask <<= 1;
	}
}


/*-------------------------------------------------------------------------*//**
 * fpga_irqmux_init()
 */
void fpga_irqmux_init(fpga_irqmux_t *mux)
{
	int i;

	HAL_ASSERT( mux );
	HAL_ASSERT( mux->mux_irqs < MAX_MUXIRQ_NUM );

	for (i=0; i<mux->mux_irqs; i++)
	{
		mux->callback[i] = NULL;
		mux->callbackdata[i] = NULL;
	}

	/* Configure IRQ handler */
	fabric_irqcallback_install(mux->irqn, (irqcallback_t)fpga_irqmux_irqhandler, mux);
	fpga_enable_interrupt(mux->irqmask);
}


/*-------------------------------------------------------------------------*//**
 * fpga_irqmux_enable_irq
 */
void fpga_irqmux_enable_irq(fpga_irqmux_t *mux, int irqn)
{
}


/*-------------------------------------------------------------------------*//**
 * fpga_irqmux_disable_irq
 */
void fpga_irqmux_disable_irq(fpga_irqmux_t *mux, int irqn)
{
}


/*-------------------------------------------------------------------------*//**
 * fpga_irqmux_clear_irq
 */
void fpga_irqmux_clear_irq(fpga_irqmux_t *mux, int irqn)
{
}


/*-------------------------------------------------------------------------*//**
 * fpga_irqmux_set_irqhandler
 */
void fpga_irqmux_set_irqhandler(fpga_irqmux_t *mux, int irqn, irqcallback_t callback, void *data)
{
	HAL_ASSERT( irqn < mux->mux_irqs );

	mux->callback[irqn] = callback;
	mux->callbackdata[irqn] = data;
}

