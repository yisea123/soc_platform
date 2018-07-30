#ifndef __GPIO_PHOTOSENSOR_H__
#define __GPIO_PHOTOSENSOR_H__

#include "photosensor.h"
#include "core_gpio.h"
#include "core_pwm.h"
#include "pwm.h"


struct gpio_photosensor_resource {
	// resource of input GPIO
	gpio_instance_t *gpiochip;	// pointer to a GPIO chip instance. Should be NULL if using MSS_GPIO.
	int gpio;			// GPIO port of a GPIO chip
	// resource of output PWM
	struct pwm_chip *pwmchip;
	pwm_id_t pwm;
};


extern int gpio_photosensor_install(struct photosensor *sensor);
extern int gpio_photosensor_drvinit(void);

#endif
