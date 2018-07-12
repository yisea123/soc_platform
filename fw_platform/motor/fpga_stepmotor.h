#ifndef __FPGA_STEPMOTOR_H__
#define __FPGA_STEPMOTOR_H__

#include "steppermotor.h"

#define MAXIMUM_STEPS		0xffffffff

typedef enum{
	STEP_MODE_FULL=1,
	STEP_MODE_HALF=2,
	STEP_MODE_16MICRO=16
} step_mode_t;


struct fpga_stepmotor_resource {
	struct steppermotor motor;
	void *mmio_base;		// registers base address
	void *ram_base;			// RAM base address
	int ram_size;			// RAM size (in bytes)
	uint32_t mask;			// motor interrupt bit mask
	int stepping;			// motor stepping multiple (should be one of 1/2/4/8/16/32)
	int clock_period;		// motor unit clock period (in nanoseconds)
	struct ramp_info *rampinfo;	// ramp table of supported speeds and speed-shifts
};

extern int fpga_stepmotor_install(struct steppermotor *motor);
extern int fpga_stepmotor_drvinit(void);

#endif
