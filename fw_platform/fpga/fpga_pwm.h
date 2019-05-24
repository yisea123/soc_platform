#ifndef __FPGA_PWM_H__
#define	__FPGA_PWM_H__

#include "pwm.h"


void SPWM_init(pwm_instance_t * pwm_inst, addr_t base_addr, uint32_t period);
void SPWM_enable(pwm_instance_t * pwm_inst, int pwm_id);
void SPWM_disable(pwm_instance_t * pwm_inst, int pwm_id);
void SPWM_set_pos_edge(pwm_instance_t * pwm_inst, int pwm_id, uint32_t pos_edge);

#endif /* __FPGA_PWM_H__ */
