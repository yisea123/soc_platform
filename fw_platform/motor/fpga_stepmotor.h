#ifndef __FPGA_STEPMOTOR_H__
#define __FPGA_STEPMOTOR_H__

#include "steppermotor.h"

#define MAXIMUM_STEPS		0xffffffff



struct fpga_stepmotor_resource {
	void *mmio_base;		// registers base address
	void *ram_base;			// RAM base address
	int ram_ramp_offset;
	int ram_size;			// RAM size (in bytes)
	int table_ramp_size;
	int table_count_size;
	int stepping;			// motor stepping multiple (should be one of 1/2/4/8/16/32)
	int clock_period;		// motor unit clock period (in nanoseconds)
	struct ramp_info *rampinfo;	// ramp table of supported speeds and speed-shifts
	uint32_t fpga_irq_mask;			// motor interrupt bit mask
	int fabric_irq;
	int pullin_speed;
};

extern int fpga_stepmotor_install(struct steppermotor *motor);
extern int fpga_stepmotor_drvinit(void);

#endif
