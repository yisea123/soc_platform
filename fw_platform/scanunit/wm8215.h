#ifndef __WM8215_H__
#define __WM8215_H__

#include "imagedigitiser.h"
#include "mss_spi.h"


struct wm8215_resource {
  	// resource of FPGA registers
	void *ctrl_base;
	void *mmio_base;
	uint32_t mask;
	// resource of SPI registers
	mss_spi_instance_t *mss_spi;
	uint32_t spi_clk_freq;
};

extern int wm8215_install(struct imagedigitiser *afe);
extern int wm8215_drvinit(void);

#endif
