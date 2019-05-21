/*
 * GPIO-based simple sensor driver
 *
 * Copyright 2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "sys_config.h"
#include "mss_gpio.h"
#include "core_gpio.h"
#include "m2sxxx.h"
#include "irqcallback.h"

#include "simplesensor.h"
#include "gpio_simplesensor.h"


static void gpio_simplesensor_eventhandler(void *device, int gpio, void *data)
{
	struct simplesensor *sensor = (struct simplesensor *)data;
	sensor->eventhandler(sensor, sensor->eventtrigger, sensor->handlerdata);
}


static int gpio_simplesensor_enable(struct simplesensor *sensor)
{
	if (!sensor)
		return -1;

	return 0;
}


static int gpio_simplesensor_disable(struct simplesensor *sensor)
{
	if (!sensor)
		return -1;

	return 0;
}


static int gpio_simplesensor_status(struct simplesensor *sensor, int *status)
{
	struct gpio_simplesensor_resource *sensor_rc;
	uint32_t gpio_inputs;
	int st;

	if (!sensor || !status)
		return -1;

	sensor_rc = (struct gpio_simplesensor_resource *)sensor->resource;

	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
		gpio_inputs = MSS_GPIO_get_inputs();
	else				//  CoreGPIO instance
		gpio_inputs = GPIO_get_inputs(sensor_rc->gpiochip);
	st = (gpio_inputs & (1 << sensor_rc->gpio)) ? 1 : 0;
	*status = (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? st : !st;

	return 0;
}


static int gpio_simplesensor_read_input(struct simplesensor *sensor, uint32_t *value)
{
	struct gpio_simplesensor_resource *sensor_rc;
	uint32_t gpio_inputs;

	if (!sensor || !value)
		return -1;

	sensor_rc = (struct gpio_simplesensor_resource *)sensor->resource;
	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
		gpio_inputs = MSS_GPIO_get_inputs();
	else				//  CoreGPIO instance
		gpio_inputs = GPIO_get_inputs(sensor_rc->gpiochip);
	*value = (gpio_inputs & (1 << sensor_rc->gpio)) ? 1 : 0;
	return 0;
}


static int gpio_simplesensor_set_event(struct simplesensor *sensor, sensor_event_t event, void (*eventhandle)(struct simplesensor *, sensor_event_t, void *), void *data)
{
	struct gpio_simplesensor_resource *sensor_rc;
	int gpio;
	uint32_t config;

	if (!sensor || !eventhandle)
		return -1;

	sensor_rc = (struct gpio_simplesensor_resource *)sensor->resource;

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


static int gpio_simplesensor_unset_event(struct simplesensor *sensor, sensor_event_t event)
{
	struct gpio_simplesensor_resource *sensor_rc;
	if (!sensor)
		return -1;

	sensor_rc = (struct gpio_simplesensor_resource *)sensor->resource;

	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
		MSS_GPIO_disable_irq((mss_gpio_id_t)sensor_rc->gpio);
	else				//  CoreGPIO instance
		GPIO_disable_irq(sensor_rc->gpiochip, (gpio_id_t)sensor_rc->gpio);

	return 0;
}


static const struct simplesensor_ops gpio_simplesensor_ops = {
	.status = gpio_simplesensor_status,
	.enable = gpio_simplesensor_enable,
	.disable = gpio_simplesensor_disable,
	.read_input = gpio_simplesensor_read_input,
	.set_event = gpio_simplesensor_set_event,
	.unset_event = gpio_simplesensor_unset_event,
};


int gpio_simplesensor_install(struct simplesensor *sensor)
{
	struct gpio_simplesensor_resource *sensor_rc;

	if (!sensor)
		return -1;
	if (!sensor->resource)
		return -1;

	sensor->ops = &gpio_simplesensor_ops;
	sensor_rc = (struct gpio_simplesensor_resource *)sensor->resource;

	/* Configure the input GPIO */
	if (!sensor_rc->gpiochip)	//  MSS GPIO instance
	{
		MSS_GPIO_config((mss_gpio_id_t)sensor_rc->gpio, MSS_GPIO_INPUT_MODE);
		mss_gpio_irqcallback_install((mss_gpio_id_t)sensor_rc->gpio, (irqcallback_t)gpio_simplesensor_eventhandler, sensor);
	}
	else				//  CoreGPIO instance
	{
		GPIO_config(sensor_rc->gpiochip, (gpio_id_t)sensor_rc->gpio, GPIO_INPUT_MODE);
		// TODO: install GPIO irqcallback here.
	}

	return 0;
}


int gpio_simplesensor_drvinit(void)
{
	return 0;
}
