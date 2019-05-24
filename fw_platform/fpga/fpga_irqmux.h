#ifndef __FPGA_IRQMUX_H__
#define	__FPGA_IRQMUX_H__

#include <stdint.h>
#include "hal.h"

#include "irqcallback.h"

#define MAX_MUXIRQ_NUM	8


typedef struct _fpga_irqmux_t
{
	addr_t		base_addr;
	int		mux_irqs;	// number of muxed IRQs
	int		irqn;		// Fabric IRQ number of an IRQ muxer
	uint32_t	irqmask;	// IRQ mask of in FPGA's general interrupt control registers
	uint32_t	status;		// dummy IRQ status register
	/* IRQ callback definition */
	irqcallback_t callback[MAX_MUXIRQ_NUM];	// IRQ callback
	void *callbackdata[MAX_MUXIRQ_NUM];
} fpga_irqmux_t;


void fpga_irqmux_init(fpga_irqmux_t *mux);
void fpga_irqmux_enable_irq(fpga_irqmux_t *mux, int irqn);
void fpga_irqmux_disable_irq(fpga_irqmux_t *mux, int irqn);
void fpga_irqmux_clear_irq(fpga_irqmux_t *mux, int irqn);
void fpga_irqmux_set_irqhandler(fpga_irqmux_t *mux, int irqn, irqcallback_t callback, void *data);

#endif /* __FPGA_IRQMUX_H__ */
