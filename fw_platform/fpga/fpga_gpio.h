#ifndef __FPGA_GPIO_H__
#define	__FPGA_GPIO_H__

#include "core_gpio.h"
#include "fpga.h"
#include "fpga_irqmux.h"
#include "irqcallback.h"

#define MAX_SGPIO_NUM	8


typedef struct _sgpio_instance_t
{
	addr_t		base_addr;
	int		reg_size;	// size of simple GPIO register in bytes
	int		bus_width;	// width of simple GPIO register IO bus in bytes
	int		irqn;		// IRQ or muxed IRQ number of an simple GPIO instance (if a sGPIO has no IRQ logic, set irqn to -1)
	uint32_t	irqmask;	// IRQ mask of in FPGA's general interrupt control registers (only used when IRQ is NOT muxed)
	fpga_irqmux_t	*irqmux;	// pointer to a FPGA IRQ muxer (if IRQ is NOT muxed, set irqmux = NULL)
	/* simple GPIO event callback definition */
	irqcallback_t callback[MAX_SGPIO_NUM];	// callback handling ADC events
	uint8_t event[MAX_SGPIO_NUM];
	void *callbackdata[MAX_SGPIO_NUM];
} sgpio_instance_t;


void SGPIO_init(sgpio_instance_t * gpio);
uint32_t SGPIO_get_inputs(sgpio_instance_t * gpio);
void SGPIO_enable_irq(sgpio_instance_t * gpio, gpio_id_t port_id);
void SGPIO_disable_irq(sgpio_instance_t * gpio, gpio_id_t port_id);
void SGPIO_clear_irq(sgpio_instance_t * gpio, gpio_id_t port_id);
void SGPIO_config_irq(sgpio_instance_t * gpio, gpio_id_t port_id, uint32_t config);
void SGPIO_set_irqcallback(sgpio_instance_t * gpio, gpio_id_t port_id, irqcallback_t callback, void *data);

#endif /* __FPGA_GPIO_H__ */
