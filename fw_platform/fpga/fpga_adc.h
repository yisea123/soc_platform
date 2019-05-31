#ifndef __FPGA_ADC_H__
#define	__FPGA_ADC_H__

#include <stdint.h>
#include "hal.h"
#include "fpga.h"
#include "adc.h"

struct fpga_adc_resource {
	addr_t		base_addr;
	int		reg_size;	// size of ADC register in bytes
	int		bus_width;	// width of ADC register IO bus in bytes
	int		irqn;		// Fabric IRQ number of an ADC instance
	uint32_t	irqmask;	// IRQ mask in FPGA's general interrupt control registers
	uint32_t	irqlogic;	// ADC channel's interrupt logic enabled flags
};


extern int fpga_adc_install(struct ad_converter *adc);
extern int fpga_adc_drvinit(void);

#endif /* __FPGA_GPIO_H__ */
