/*
 * FPGA-based simple PWM driver
 *
 * Copyright 2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include <stdint.h>
#include "hal_assert.h"
#include "fpga_io.h"
#include "fpga.h"

#include "fpga_pwm.h"


/*-------------------------------------------------------------------------*//**
 * SPWM_init()
 */
void SPWM_init(pwm_instance_t * pwm_inst, addr_t base_addr, uint32_t period)
{
	uint32_t value;
	int i;

	pwm_inst->address = base_addr;
	fpga_writel(0u, (char *)base_addr + FPGA_REG_PWM_CONTROL);

	value = BITS_11_TO_4(period);	// use only higher bit[11:4] and ignore lower bit[3:0]
	if ((period & 0xf) > 7)		// round-up lower bit[3:0]
		value += 1;
	fpga_writel(value, (char *)base_addr + FPGA_REG_PWM_PERIOD);

	/* Set positive edge to 0 for all PWMs. */
	for (i = 0; i < 8; i++)
		fpga_writel(0u, (char *)base_addr + FPGA_REG_PWM_DUTY_1 + sizeof(uint32_t) * i);
}

/*-------------------------------------------------------------------------*//**
 * SPWM_enable()
 */
void SPWM_enable(pwm_instance_t * pwm_inst, int pwm_id)
{
	uint32_t mask;

	/* Assertion will ensure correct PWM output has been selected. */
	HAL_ASSERT( pwm_id >= 0 );
	HAL_ASSERT( pwm_id <= 7 );

	mask = FPGA_REG_PWM_MASK(pwm_id);
	fpga_update_lbits((char *)pwm_inst->address + FPGA_REG_PWM_CONTROL, mask, mask);
}


/*-------------------------------------------------------------------------*//**
 * SPWM_disable()
 */
void SPWM_disable(pwm_instance_t * pwm_inst, int pwm_id)
{
	uint32_t mask;

	/* Assertion will ensure correct PWM output has been selected. */
	HAL_ASSERT( pwm_id >= 0 );
	HAL_ASSERT( pwm_id <= 7 );

	mask = FPGA_REG_PWM_MASK(pwm_id);
	fpga_update_lbits((char *)pwm_inst->address + FPGA_REG_PWM_CONTROL, mask, 0);
}


/*-------------------------------------------------------------------------*//**
 * SPWM_set_pos_edge()
 */
void SPWM_set_pos_edge(pwm_instance_t * pwm_inst, int pwm_id, uint32_t pos_edge)
{
	uint32_t value;

	/* Assertion will ensure correct PWM output has been selected. */
	HAL_ASSERT( pwm_id >= 0 );
	HAL_ASSERT( pwm_id <= 7 );

	value = BITS_11_TO_4(pos_edge);	// use only higher bit[11:4] and ignore lower bit[3:0]
	if ((pos_edge & 0xf) > 7)	// round-up lower bit[3:0]
		value += 1;
	fpga_writel(value, (char *)pwm_inst->address + FPGA_REG_PWM_DUTY_1 + sizeof(uint32_t) * pwm_id);
}

