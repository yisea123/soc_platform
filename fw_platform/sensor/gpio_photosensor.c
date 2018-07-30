/*
 * GPIO-based photosensor driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "sys_config.h"
#include "mss_gpio.h"
#include "core_gpio.h"
#include "m2sxxx.h"
#include "irqcallback.h"

#include "photosensor.h"
#include "gpio_photosensor.h"


static void gpio_photosensor_eventhandler(void *device, int gpio, void *data)
{
	struct photosensor *sensor = (struct photosensor *)data;
	sensor->eventhandle(sensor, sensor->eventtrigger, sensor->handledata);
}


static int gpio_photosensor_enable(struct photosensor *sensor)
{
	struct gpio_photosensor_resource *sensor_rc;

	if (!sensor)
		return -1;

	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;
	PWM_enable(&sensor_rc->pwmchip->pwm_inst, sensor_rc->pwm);
	return 0;
}


static int gpio_photosensor_disable(struct photosensor *sensor)
{
	struct gpio_photosensor_resource *sensor_rc;

	if (!sensor)
		return -1;

	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;
	PWM_disable(&sensor_rc->pwmchip->pwm_inst, sensor_rc->pwm);
	return 0;
}


static int gpio_photosensor_status(struct photosensor *sensor, int *status)
{
	struct gpio_photosensor_resource *sensor_rc;
	uint32_t gpio_inputs;

	if (!sensor || !status)
		return -1;

	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;
	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
		gpio_inputs = MSS_GPIO_get_inputs();
	else				//  CoreGPIO instance
		gpio_inputs = GPIO_get_inputs(sensor_rc->gpiochip);
	*status = (gpio_inputs & (1 << sensor_rc->gpio)) ? 1 : 0;
	return 0;
}


static int gpio_photosensor_read_input(struct photosensor *sensor, uint32_t *value)
{
	struct gpio_photosensor_resource *sensor_rc;
	uint32_t gpio_inputs;

	if (!sensor || !value)
		return -1;

	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;
	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
		gpio_inputs = MSS_GPIO_get_inputs();
	else				//  CoreGPIO instance
		gpio_inputs = GPIO_get_inputs(sensor_rc->gpiochip);
	*value = (gpio_inputs & (1 << sensor_rc->gpio)) ? 1 : 0;
	return 0;
}


static int gpio_photosensor_config(struct photosensor *sensor, const struct photosensor_config *config)
{
	struct gpio_photosensor_resource *sensor_rc;
	int duty;
	uint32_t duty_cnt;

	if (!sensor || !config)
		return -1;

	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;
	duty = BRIGHTNESS_TO_DUTY(config->led_brightness, sensor_rc->pwmchip->period);
	duty_cnt = PWM_DUTY(sensor_rc->pwmchip, duty);
	PWM_set_edges(&sensor_rc->pwmchip->pwm_inst, sensor_rc->pwm, 0, duty_cnt);

	return 0;
}


static int gpio_photosensor_set_event(struct photosensor *sensor, sensor_event_t event, void (*eventhandle)(struct photosensor *, sensor_event_t, void *), void *data)
{
	struct gpio_photosensor_resource *sensor_rc;
	int gpio;
	uint32_t config;

	if (!sensor || !eventhandle)
		return -1;

	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;

	gpio = sensor_rc->gpio;
	if (!sensor_rc->gpiochip)	// MSS GPIO instance
	{
		MSS_GPIO_disable_irq((mss_gpio_id_t)gpio);
		MSS_GPIO_clear_irq((mss_gpio_id_t)gpio);

		config = MSS_GPIO_INPUT_MODE;
		switch (event)
		{
		case SENSOR_EV_NONE:
			return 0;
		case SENSOR_EV_DETECTED:
			config |= (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? \
					MSS_GPIO_IRQ_EDGE_POSITIVE : MSS_GPIO_IRQ_EDGE_NEGATIVE;
			break;
		case SENSOR_EV_UNDETECTED:
			config |= (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? \
					MSS_GPIO_IRQ_EDGE_NEGATIVE : MSS_GPIO_IRQ_EDGE_POSITIVE;
			break;
		case SENSOR_EV_EITHER:
			config |= MSS_GPIO_IRQ_EDGE_BOTH;
			break;
		}

		MSS_GPIO_config((mss_gpio_id_t)gpio, config);
		MSS_GPIO_enable_irq((mss_gpio_id_t)gpio);
	}
	else				//  CoreGPIO instance
	{
		GPIO_disable_irq(sensor_rc->gpiochip, (gpio_id_t)gpio);
		GPIO_clear_irq(sensor_rc->gpiochip, (gpio_id_t)gpio);

		config = GPIO_INPUT_MODE;
		switch (event)
		{
		case SENSOR_EV_NONE:
			return 0;
		case SENSOR_EV_DETECTED:
			config |= (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? \
					GPIO_IRQ_EDGE_POSITIVE : GPIO_IRQ_EDGE_NEGATIVE;
			break;
		case SENSOR_EV_UNDETECTED:
			config |= (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? \
					GPIO_IRQ_EDGE_NEGATIVE : GPIO_IRQ_EDGE_POSITIVE;
			break;
		case SENSOR_EV_EITHER:
			config |= GPIO_IRQ_EDGE_BOTH;
			break;
		}

		GPIO_config(sensor_rc->gpiochip, (gpio_id_t)gpio, config);
		GPIO_enable_irq(sensor_rc->gpiochip, (gpio_id_t)gpio);
	}
	return 0;
}


static int gpio_photosensor_unset_event(struct photosensor *sensor, sensor_event_t event)
{
	struct gpio_photosensor_resource *sensor_rc;
	if (!sensor)
		return -1;

	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;
	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
		MSS_GPIO_disable_irq((mss_gpio_id_t)sensor_rc->gpio);
	else				//  CoreGPIO instance
		GPIO_disable_irq(sensor_rc->gpiochip, (gpio_id_t)sensor_rc->gpio);

	return 0;
}


static const struct photosensor_ops gpio_photosensor_ops = {
	.config = gpio_photosensor_config,
	.status = gpio_photosensor_status,
	.enable = gpio_photosensor_enable,
	.disable = gpio_photosensor_disable,
	.read_input = gpio_photosensor_read_input,
	.set_event = gpio_photosensor_set_event,
	.unset_event = gpio_photosensor_unset_event,
};


int gpio_photosensor_install(struct photosensor *sensor)
{
	struct gpio_photosensor_resource *sensor_rc;

	if (!sensor)
		return -1;
	if (!sensor->resource)
		return -1;

	sensor->ops = &gpio_photosensor_ops;
	sensor_rc = (struct gpio_photosensor_resource *)sensor->resource;

	/* Configure the input GPIO */
	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
	{
		MSS_GPIO_config((mss_gpio_id_t)sensor_rc->gpio, MSS_GPIO_INPUT_MODE);
		mss_gpio_irqcallback_install((mss_gpio_id_t)sensor_rc->gpio, (irqcallback_t)gpio_photosensor_eventhandler, sensor);
	}
	else				//  CoreGPIO instance
	{
		GPIO_config(sensor_rc->gpiochip, (gpio_id_t)sensor_rc->gpio, GPIO_INPUT_MODE);
		// TODO: install GPIO irqcallback here.
	}

	/* Configure the output PWM */

	return 0;
}


int gpio_photosensor_drvinit(void)
{
	return 0;
}
