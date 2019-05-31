/*
 * FPAG-based analog and digital photosensor driver
 *
 * Copyright 2018,2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "sys_config.h"
#include "m2sxxx.h"
#include "irqcallback.h"

#include "photosensor.h"
#include "fpga_photosensor.h"
#include "fpga.h"
#include "fpga_gpio.h"
#include "fpga_pwm.h"
#include "adc.h"


static void fpga_photosensor_eventhandler(void *device, int chan, void *data)
{
	struct photosensor *sensor = (struct photosensor *)data;
	sensor->eventhandle(sensor, sensor->eventtrigger, sensor->handledata);
}


static int fpga_photosensor_enable(struct photosensor *sensor)
{
	struct fpga_photosensor_resource *sensor_rc;

	if (!sensor)
		return -1;

	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;
	SPWM_enable(&sensor_rc->pwmchip->pwm_inst, sensor_rc->pwm);
	return 0;
}


static int fpga_photosensor_disable(struct photosensor *sensor)
{
	struct fpga_photosensor_resource *sensor_rc;

	if (!sensor)
		return -1;

	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;
	SPWM_disable(&sensor_rc->pwmchip->pwm_inst, sensor_rc->pwm);
	return 0;
}


static int fpga_photosensor_status(struct photosensor *sensor, int *status)
{
	struct fpga_photosensor_resource *sensor_rc;

	if (!sensor || !status)
		return -1;

	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;
	if (sensor->type == PHOTOSENSOR_DIGITAL)
	{	// SIMPLE GPIO input
		uint32_t fpga_inputs;
		fpga_inputs = SGPIO_get_inputs((sgpio_instance_t *)sensor_rc->inputdev);
		*status = (fpga_inputs & (1 << sensor_rc->inputchan)) ? 1 : 0;
	}
	else
	{
		// FPGA ADC input
		int rs = adc_read_compare((struct ad_converter *)sensor_rc->inputdev, sensor_rc->inputchan, (uint32_t *)status);
		if (rs != 0)
			return -1;
	}
	if(sensor->status_mapping == SENSOR_ST_DETETED_IS_LOWLEVEL)
		*status = !(*status);
	return 0;
}


static int fpga_photosensor_read_input(struct photosensor *sensor, uint32_t *value)
{
	struct fpga_photosensor_resource *sensor_rc;
	uint32_t fpga_inputs;

	if (!sensor || !value)
		return -1;

	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;
	if (sensor->type == PHOTOSENSOR_DIGITAL)
	{	//  SIMPLE GPIO input
		fpga_inputs = SGPIO_get_inputs((sgpio_instance_t *)sensor_rc->inputdev);
		*value = (fpga_inputs & (1 << sensor_rc->inputchan)) ? 1 : 0;
	}
	else
	{
		// FPGA ADC input
		int rs = adc_read_average((struct ad_converter *)sensor_rc->inputdev, sensor_rc->inputchan, value);
		if (rs != 0)
			return -1;
	}
	return 0;
}


static int fpga_photosensor_config(struct photosensor *sensor, const struct photosensor_config *config)
{
	struct fpga_photosensor_resource *sensor_rc;
	int duty;
	uint32_t duty_cnt;

	if (!sensor || !config)
		return -1;

	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;
	duty = BRIGHTNESS_TO_DUTY(config->led_brightness, sensor_rc->pwmchip->period);
	duty_cnt = PWM_DUTY(sensor_rc->pwmchip, duty);
	SPWM_set_pos_edge(&sensor_rc->pwmchip->pwm_inst, sensor_rc->pwm, duty_cnt);

	if (sensor->type == PHOTOSENSOR_ANALOG)
	{
		struct ad_converter *adc = (struct ad_converter *)sensor_rc->inputdev;

		adc_set_threshold(adc, sensor_rc->inputchan, config->compare_threshold);
	}
	return 0;
}


static int fpga_photosensor_set_event(struct photosensor *sensor, sensor_event_t event, void (*eventhandler)(struct photosensor *, sensor_event_t, void *), void *data)
{
	struct fpga_photosensor_resource *sensor_rc;

	if (!sensor || !eventhandler)
		return -1;

	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;

	if (sensor->type == PHOTOSENSOR_DIGITAL)
	{
		sgpio_instance_t *gpiochip = (sgpio_instance_t *)sensor_rc->inputdev;
		gpio_id_t gpio = (gpio_id_t)sensor_rc->inputchan;
		uint32_t config;

		SGPIO_disable_irq(gpiochip, gpio);
		SGPIO_clear_irq(gpiochip, gpio);

		switch (event)
		{
		case SENSOR_EV_NONE:
			return 0;	// for SENSOR_EV_NONE, just return with this GPIO interrupt diabled !
		case SENSOR_EV_DETECTED:
			config = (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? \
					FPGA_IRQ_MODE_RISING_EDGE : FPGA_IRQ_MODE_FALLING_EDGE;
			break;
		case SENSOR_EV_UNDETECTED:
			config = (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? \
					FPGA_IRQ_MODE_FALLING_EDGE : FPGA_IRQ_MODE_RISING_EDGE;
			break;
		case SENSOR_EV_EITHER:
			config = FPGA_IRQ_MODE_BOTH_EDGES;
			break;
		}
		SGPIO_config_irq(gpiochip, gpio, config);
		SGPIO_enable_irq(gpiochip, gpio);
	}
	else
	{
		struct ad_converter *adc = (struct ad_converter *)sensor_rc->inputdev;
		adc_event_t adc_evt;		

		switch (event)
		{
		case SENSOR_EV_NONE:
			return 0;	// for SENSOR_EV_NONE, just return with this ADC event trigger diabled !
		case SENSOR_EV_DETECTED:
			adc_evt = (sensor->status_mapping == SENSOR_ST_DETETED_IS_ABOVE_THRESHOLD) ?\
					ADC_EVT_CHANGE_RISING : ADC_EVT_CHANGE_FALLING;
			break;
		case SENSOR_EV_UNDETECTED:
			adc_evt = (sensor->status_mapping == SENSOR_ST_DETETED_IS_BELOW_THRESHOLD) ?\
					ADC_EVT_CHANGE_FALLING : ADC_EVT_CHANGE_RISING;
			break;
		case SENSOR_EV_EITHER:
			adc_evt = ADC_EVT_CHANGE_EITHER;
			break;
		}
		adc_set_event(adc, sensor_rc->inputchan, adc_evt);
	}

	return 0;
}


static int fpga_photosensor_unset_event(struct photosensor *sensor, sensor_event_t event)
{
	struct fpga_photosensor_resource *sensor_rc;

	if (!sensor)
		return -1;

	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;
	if (sensor->type == PHOTOSENSOR_DIGITAL)
	{
		sgpio_instance_t *gpiochip = (sgpio_instance_t *)sensor_rc->inputdev;
		gpio_id_t gpio = (gpio_id_t)sensor_rc->inputchan;

		SGPIO_disable_irq(gpiochip, gpio);
		SGPIO_clear_irq(gpiochip, gpio);
	}
	else
	{
		struct ad_converter *adc = (struct ad_converter *)sensor_rc->inputdev;
		adc_unset_event(adc, sensor_rc->inputchan);
	}

	return 0;
}


static const struct photosensor_ops fpga_photosensor_ops = {
	.config = fpga_photosensor_config,
	.status = fpga_photosensor_status,
	.enable = fpga_photosensor_enable,
	.disable = fpga_photosensor_disable,
	.read_input = fpga_photosensor_read_input,
	.set_event = fpga_photosensor_set_event,
	.unset_event = fpga_photosensor_unset_event,
};


int fpga_photosensor_install(struct photosensor *sensor)
{
	struct fpga_photosensor_resource *sensor_rc;
	int rs;

	if (!sensor)
		return -1;
	if (!sensor->resource)
		return -1;

	sensor->ops = &fpga_photosensor_ops;
	sensor_rc = (struct fpga_photosensor_resource *)sensor->resource;

	if (!sensor_rc->inputdev)	// check sensor input
		return -1;

	/* Configure the input */
	if (sensor->type == PHOTOSENSOR_DIGITAL)
	{
		sgpio_instance_t *gpio = (sgpio_instance_t *)sensor_rc->inputdev;
		SGPIO_set_irqcallback(gpio, (gpio_id_t)sensor_rc->inputchan, (irqcallback_t)fpga_photosensor_eventhandler, sensor);
	}
	else
	{
		// Analog sensor
		struct ad_converter *adc = (struct ad_converter *)sensor_rc->inputdev;
		rs = adc_set_callback(adc, sensor_rc->inputchan, fpga_photosensor_eventhandler, sensor);
	}

	/* Configure the output PWM */

	return 0;
}


int fpga_photosensor_drvinit(void)
{
	return 0;
}
