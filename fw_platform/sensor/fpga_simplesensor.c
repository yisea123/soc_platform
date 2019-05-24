/*
 * FPGA-based simple sensor driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "sys_config.h"
#include "m2sxxx.h"
#include "irqcallback.h"

#include "simplesensor.h"
#include "fpga_simplesensor.h"

#include "fpga.h"
#include "fpga_gpio.h"


static void fpga_simplesensor_eventhandler(void *device, int gpio, void *data)
{
	struct simplesensor *sensor = (struct simplesensor *)data;
	sensor->eventhandler(sensor, sensor->eventtrigger, sensor->handlerdata);
}


static int fpga_simplesensor_enable(struct simplesensor *sensor)
{
	if (!sensor)
		return -1;

	return 0;
}


static int fpga_simplesensor_disable(struct simplesensor *sensor)
{
	if (!sensor)
		return -1;

	return 0;
}


static int fpga_simplesensor_status(struct simplesensor *sensor, int *status)
{
	struct fpga_simplesensor_resource *sensor_rc;
	uint32_t fpga_inputs;
	int st;

	if (!sensor || !status)
		return -1;

	sensor_rc = (struct fpga_simplesensor_resource *)sensor->resource;

	fpga_inputs = SGPIO_get_inputs(sensor_rc->gpiochip);
	st = (fpga_inputs & (1 << sensor_rc->gpio)) ? 1 : 0;
	*status = (sensor->status_mapping == SENSOR_ST_DETETED_IS_HIGHLEVEL) ? st : !st;

	return 0;
}


static int fpga_simplesensor_read_input(struct simplesensor *sensor, uint32_t *value)
{
	struct fpga_simplesensor_resource *sensor_rc;
	uint32_t fpga_inputs;

	if (!sensor || !value)
		return -1;

	sensor_rc = (struct fpga_simplesensor_resource *)sensor->resource;

	fpga_inputs = SGPIO_get_inputs(sensor_rc->gpiochip);
	*value = (fpga_inputs & (1 << sensor_rc->gpio)) ? 1 : 0;

	return 0;
}


static int fpga_simplesensor_set_event(struct simplesensor *sensor, sensor_event_t event, void (*eventhandle)(struct simplesensor *, sensor_event_t, void *), void *data)
{
	struct fpga_simplesensor_resource *sensor_rc;
	int chan;
	uint32_t config;

	if (!sensor || !eventhandle)
		return -1;

	sensor_rc = (struct fpga_simplesensor_resource *)sensor->resource;

	chan = sensor_rc->gpio;
	SGPIO_disable_irq(sensor_rc->gpiochip, (gpio_id_t)chan);
	SGPIO_clear_irq(sensor_rc->gpiochip, (gpio_id_t)chan);

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

	SGPIO_config_irq(sensor_rc->gpiochip, (gpio_id_t)chan, config);
	SGPIO_enable_irq(sensor_rc->gpiochip, (gpio_id_t)chan);

	return 0;
}


static int fpga_simplesensor_unset_event(struct simplesensor *sensor, sensor_event_t event)
{
	struct fpga_simplesensor_resource *sensor_rc;
	int chan;

	if (!sensor)
		return -1;

	sensor_rc = (struct fpga_simplesensor_resource *)sensor->resource;

	chan = sensor_rc->gpio;
	SGPIO_disable_irq(sensor_rc->gpiochip, (gpio_id_t)chan);
	SGPIO_clear_irq(sensor_rc->gpiochip, (gpio_id_t)chan);

	return 0;
}


static const struct simplesensor_ops fpga_simplesensor_ops = {
	.status = fpga_simplesensor_status,
	.enable = fpga_simplesensor_enable,
	.disable = fpga_simplesensor_disable,
	.read_input = fpga_simplesensor_read_input,
	.set_event = fpga_simplesensor_set_event,
	.unset_event = fpga_simplesensor_unset_event,
};


int fpga_simplesensor_install(struct simplesensor *sensor)
{
	struct fpga_simplesensor_resource *sensor_rc;

	if (!sensor)
		return -1;
	if (!sensor->resource)
		return -1;

	sensor->ops = &fpga_simplesensor_ops;
	sensor_rc = (struct fpga_simplesensor_resource *)sensor->resource;

	/* Configure the input */
	if (!sensor_rc->gpiochip)	// check digital sensor input
			return -1;
	fabric_irqcallback_install((gpio_id_t)sensor_rc->gpio, (irqcallback_t)fpga_simplesensor_eventhandler, sensor);

	return 0;
}


int fpga_simplesensor_drvinit(void)
{
	return 0;
}
