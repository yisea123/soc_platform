#ifndef __FPGADRV_H__
#define __FPGADRV_H__

#include <stdint.h>
#include <stdbool.h>

struct fpga_resource {
	void *ctrl_reg_base;
	void *ints_reg_base;
	uint32_t mclk_frequency;
};


extern int fpga_install(const struct fpga_resource *);
extern void fpga_reset(void);
extern uint32_t fpga_get_version(void);
extern void fpga_enable_interrupt(uint32_t);
extern void fpga_disable_interrupt(uint32_t);
extern void fpga_clear_interrupt(uint32_t);
extern void fpga_enable(uint32_t on_off);
extern uint32_t fpga_get_product_type(void);
#endif
