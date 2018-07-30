#ifndef __IRQCALLBACK_H__
#define __IRQCALLBACK_H__

#include <stdint.h>


typedef	void *(*irqcallback_t)(void *, int , void *);

int mss_gpio_irqcallback_install(mss_gpio_id_t, irqcallback_t, void *);
int mss_gpio_irqcallback_remove(mss_gpio_id_t);

int fabric_irqcallback_install(int, irqcallback_t, void *);
int fabric_irqcallback_remove(int);

#endif /* __IRQCALLBACK_H__ */