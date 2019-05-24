#ifndef __GPIO_PHOTOSENSOR_H__
#define __GPIO_PHOTOSENSOR_H__

#include "photosensor.h"
#include "pwm.h"
#include "fpga_pwm.h"
#include "fpga_gpio.h"
#include "adc.h"


struct fpga_photosensor_resource {
	// resource of input GPIO or ADC
	void *const inputdev;		// generic pointer to an input device (simple GPIO or ADC)
	int inputchan;			// channel(port) of an input device (simple GPIO or ADC)
	// resource of output PWM
	struct pwm_chip *pwmchip;
	int pwm;
};


extern int fpga_photosensor_install(struct photosensor *sensor);
extern int fpga_photosensor_drvinit(void);

#endif
