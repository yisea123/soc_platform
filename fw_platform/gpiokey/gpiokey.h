#ifndef __GPIOKEY_H__
#define __GPIOKEY_H__

#include "core_gpio.h"
#include "mss_gpio.h"
#include "irqcallback.h"

/* low-lever hardware gpio status mapping to logical status */
typedef enum {
  	GPIOKEY_TYPE_IRQ_EDGE_LEVEL_HIGH,
	GPIOKEY_TYPE_IRQ_EDGE_LEVEL_LOW,	
	GPIOKEY_TYPE_IRQ_EDGE_POSITIVE,
	GPIOKEY_TYPE_IRQ_EDGE_NEGATIVE,
	GPIOKEY_TYPE_IRQ_EDGE_BOTH,
	GPIOKEY_TYPE_LEVEL_HIGH,
	GPIOKEY_TYPE_LEVEL_LOW
} gpiokey_type_t;

typedef enum {
  	GPIOKEY_ST_NOT_ACTIVE,
  	GPIOKEY_ST_ACTIVE,
} gpiokey_status_t;

struct gpiokey_resource {
	// resource of input GPIO
	gpio_instance_t *gpiochip;	// pointer to a GPIO chip instance. Should be NULL if using MSS_GPIO.
	int gpio;			// GPIO port of a GPIO chip
	gpiokey_type_t gpiokey_type;
	gpiokey_status_t gpiokey_status;
};

struct gpiokey {
	const void *resource;				// pointer to the resource block of an imagesensor instance
	int (*const install)(struct gpiokey *key);	// pointer to the device installation function
	int (*status_is_active)(struct gpiokey *key);
	void (*status_clear)(struct gpiokey *key);
};

extern int gpiokey_install_devices(void);
extern int gpiokey_install(struct gpiokey *pgpiokey);
extern int gpiokey_is_active(struct gpiokey *pgpiokey);
#endif
