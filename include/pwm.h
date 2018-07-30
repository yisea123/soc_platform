#ifndef __PWM_H__
#define __PWM_H__

#include <stdint.h>
#include "core_pwm.h"


struct pwm_chip {
	pwm_instance_t pwm_inst;
	addr_t base_addr;	// base address of the PWM instance
	int apb_dwidth;		// APB bus width
	int pclk_period;	// PWM instance system clock (PCLK) period in nanosecond (ns)
	int prescale;		// precale of PWM instance
	int period;		// PWM period in nanosecond (ns)
};


#define PWM_GRANULARITY(pclk, prescale)		((pclk)*((prescale)+1))
/* macros converting period/duty in nanoseconds to PWM api parameters */
#define PWM_NS_TO_CNT(pclk, prescale, ns)	(uint32_t)(((ns)/PWM_GRANULARITY(pclk, prescale))-1)
#define PWM_PERIOD(pwmchip, period)		PWM_NS_TO_CNT((pwmchip)##->pclk_period, (pwmchip)##->prescale, period)
#define PWM_DUTY(pwmchip, duty)			PWM_NS_TO_CNT((pwmchip)##->pclk_period, (pwmchip)##->prescale, duty)

#endif /* __PWM_H__ */