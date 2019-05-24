#ifndef __FPGA_SIMPLESENSOR_H__
#define __FPGA_SIMPLESENSOR_H__

#include "simplesensor.h"
#include "fpga_gpio.h"


struct fpga_simplesensor_resource {
	// resource of input GPIO
	sgpio_instance_t *gpiochip;	// pointer to a simple GPIO chip instance
	int gpio;			// GPIO port of a GPIO chip
};


extern int fpga_simplesensor_install(struct simplesensor *sensor);
extern int fpga_simplesensor_drvinit(void);

#endif /* __FPGA_SIMPLESENSOR_H__ */
