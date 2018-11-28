#include "gpiokey.h"

extern struct gpiokey gpiokey_list[];
extern const int gpiokey_num ;

static void gpiokey_irq_handler(void *device, int gpio, void *data)
{
  	struct gpiokey_resource *pgpiokey_rc = (struct gpiokey_resource *)data;
	pgpiokey_rc->gpiokey_status = GPIOKEY_ST_ACTIVE;
}

void gpiokey_init(struct gpiokey_resource *pgpiokey_rc)
{
	pgpiokey_rc->gpiokey_status = GPIOKEY_ST_NOT_ACTIVE;
	
	if (!pgpiokey_rc->gpiochip)	//  MSS GPIO instance
	{
		MSS_GPIO_disable_irq((mss_gpio_id_t)pgpiokey_rc->gpio);
		MSS_GPIO_clear_irq((mss_gpio_id_t)pgpiokey_rc->gpio);
		
		switch(pgpiokey_rc->gpiokey_type)
		{
		case GPIOKEY_TYPE_IRQ_EDGE_LEVEL_HIGH:
		  	MSS_GPIO_config((mss_gpio_id_t)pgpiokey_rc->gpio, MSS_GPIO_INPUT_MODE|MSS_GPIO_IRQ_LEVEL_HIGH);
			break;
		case GPIOKEY_TYPE_IRQ_EDGE_LEVEL_LOW:
		  	MSS_GPIO_config((mss_gpio_id_t)pgpiokey_rc->gpio, MSS_GPIO_INPUT_MODE|MSS_GPIO_IRQ_LEVEL_LOW);
		 	break;
		case GPIOKEY_TYPE_IRQ_EDGE_POSITIVE:		  
		  	MSS_GPIO_config((mss_gpio_id_t)pgpiokey_rc->gpio, MSS_GPIO_INPUT_MODE|MSS_GPIO_IRQ_EDGE_POSITIVE);
		  	break;
		case GPIOKEY_TYPE_IRQ_EDGE_NEGATIVE:
			MSS_GPIO_config((mss_gpio_id_t)pgpiokey_rc->gpio, MSS_GPIO_INPUT_MODE|MSS_GPIO_IRQ_EDGE_NEGATIVE);
			break;
		case GPIOKEY_TYPE_IRQ_EDGE_BOTH:
		  	MSS_GPIO_config((mss_gpio_id_t)pgpiokey_rc->gpio, MSS_GPIO_INPUT_MODE|MSS_GPIO_IRQ_EDGE_BOTH);
            break;
        case GPIOKEY_TYPE_LEVEL_LOW:
        case GPIOKEY_TYPE_LEVEL_HIGH:
		  	MSS_GPIO_config((mss_gpio_id_t)pgpiokey_rc->gpio, MSS_GPIO_INPUT_MODE);
		  	return;
		default:
		  	return;
		}
		mss_gpio_irqcallback_install((mss_gpio_id_t)pgpiokey_rc->gpio, (irqcallback_t)gpiokey_irq_handler, (void *)pgpiokey_rc);

		MSS_GPIO_enable_irq((mss_gpio_id_t)pgpiokey_rc->gpio);
		
	}
	else				//  CoreGPIO instance
	{
		GPIO_disable_irq(pgpiokey_rc->gpiochip, (gpio_id_t)pgpiokey_rc->gpio);
		GPIO_clear_irq(pgpiokey_rc->gpiochip, (gpio_id_t)pgpiokey_rc->gpio);
		
		GPIO_config(pgpiokey_rc->gpiochip, (gpio_id_t)pgpiokey_rc->gpio, GPIO_INPUT_MODE|GPIO_IRQ_EDGE_NEGATIVE);
		GPIO_enable_irq(pgpiokey_rc->gpiochip, (gpio_id_t)pgpiokey_rc->gpio);
		// TODO: install GPIO irqcallback here.
	}
}

int gpiokey_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<gpiokey_num; i++)
	{
		if (gpiokey_list[i].install != NULL)
		{
			rs = gpiokey_list[i].install(&gpiokey_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}

int gpiokey_is_active(struct gpiokey *pgpiokey)
{
	struct gpiokey_resource *pgpiokey_rc = (struct gpiokey_resource *)pgpiokey->resource;
	
	if(pgpiokey_rc->gpiokey_type<=GPIOKEY_TYPE_IRQ_EDGE_BOTH)
  		return ((pgpiokey_rc->gpiokey_status == GPIOKEY_ST_ACTIVE)?1:0);
	else
	{
		uint32_t gpio_inputs, val;
		
		gpio_inputs = MSS_GPIO_get_inputs();
		val = ((gpio_inputs & (1 << pgpiokey_rc->gpio)) ? 1 : 0);
		if((pgpiokey_rc->gpiokey_type==GPIOKEY_TYPE_LEVEL_HIGH) && (val))
			pgpiokey_rc->gpiokey_status = GPIOKEY_ST_ACTIVE;
		else if((pgpiokey_rc->gpiokey_type==GPIOKEY_TYPE_LEVEL_LOW) && (!val))
			pgpiokey_rc->gpiokey_status = GPIOKEY_ST_ACTIVE;
		else
			pgpiokey_rc->gpiokey_status = GPIOKEY_ST_NOT_ACTIVE;

		return ((pgpiokey_rc->gpiokey_status == GPIOKEY_ST_ACTIVE)?1:0);
			
	}
		
}

void gpiokey_status_clear(struct gpiokey *pgpiokey)
{
	struct gpiokey_resource *pgpiokey_rc = (struct gpiokey_resource *)pgpiokey->resource;

	if(pgpiokey_rc->gpiokey_type<=GPIOKEY_TYPE_IRQ_EDGE_BOTH)
		pgpiokey_rc->gpiokey_status=GPIOKEY_ST_NOT_ACTIVE;
}

int gpiokey_install(struct gpiokey *pgpiokey)
{
	if (!pgpiokey)
		return -1;
	if (!pgpiokey->resource)
		return -1;

	pgpiokey->status_is_active = gpiokey_is_active;
	pgpiokey->status_clear = gpiokey_status_clear;

	gpiokey_init((struct gpiokey_resource *)pgpiokey->resource);

	return 0;
}
