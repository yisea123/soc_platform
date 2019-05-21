#ifndef __GPIO_SIMPLESENSOR_H__
#define __GPIO_SIMPLESENSOR_H__

#include "simplesensor.h"
#include "core_gpio.h"


struct gpio_simplesensor_resource {
	// resource of input GPIO
	gpio_instance_t *gpiochip;	// pointer to a GPIO chip instance. Should be NULL if using MSS_GPIO.
	int gpio;			// GPIO port of a GPIO chip
};


extern int gpio_simplesensor_install(struct simplesensor *sensor);
extern int gpio_simplesensor_drvinit(void);

#endif /* __GPIO_SIMPLESENSOR_H__ */
