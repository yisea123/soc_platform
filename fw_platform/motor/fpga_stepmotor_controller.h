#ifndef __FPGA_STEPMOTOR_CONTROLLER_H__
#define __FPGA_STEPMOTOR_CONTROLLER_H__

#include "steppermotor.h"
#include "fpga_timer.h"

#define MAXIMUM_STEPS		0xffffffff



#define FPGA_REG_MOTOR_CONTROLLER_CONTROL		0x0000

/* bits definition of FPGA_REG_MOTORx_CONTROL */
#define FPGA_REG_MOTOR_CONTROLLER_RUN		BIT(0)
#define FPGA_REG_MOTOR_CONTROLLER_DIRECTION	BIT(1)
#define FPGA_REG_MOTOR_CONTROLLER_EN		BIT(2)
#define FPGA_REG_MOTOR_CONTROLLER_MODE		(7 << 3)
#define FPGA_REG_MOTOR_CONTROLLER_HOLD		BIT(6)
#define FPGA_REG_MOTOR_CONTROLLER_SEL		BIT(7)

#define FPGA_MOTOR_CONTROLLER_DIRECTION(x)	(x<<1) 
#define FPGA_MOTOR_CONTROLLER_EN(x)		(x<<2)
#define FPGA_MOTOR_CONTROLLER_MODE(x)	(x<<3)
#define FPGA_MOTOR_CONTROLLER_HOLD(x)	(x<<6)
#define FPGA_MOTOR_CONTROLLER_SEL(x)	(x<<7)
#define FPGA_REG_MOTOR_CONTROLLER_CONTROL_MASK 		0xff


/* definition of steppermotor controller ctrl status */
typedef enum{
	STEPPERMOTOR_CONTROLLER_RUNNING_INIT,
	STEPPERMOTOR_CONTROLLER_RUNNING_ACC,
	STEPPERMOTOR_CONTROLLER_RUNNING_CONST,
	STEPPERMOTOR_CONTROLLER_RUNNING_DEC,
	STEPPERMOTOR_CONTROLLER_RUNNING_HOLD	
}steppermotor_controller_ctrl_status_t;

struct fpga_stepmotor_controller_ctrl {
	int steps_to_run;
	int steps_const;
	motion_dir dir;				// motor motion direction
	struct speed_info *speedinfo;
	struct motor_speedtable *speedtable;
	int speed_index;
	const uint32_t * ptr_ramp;
	uint32_t steps_count;
	uint32_t steps_count_tmp;
	steppermotor_controller_ctrl_status_t ctrl_status;
};

struct fpga_stepmotor_controller_resource {
	void *mmio_base;		// registers base address
	struct fpga_timer *pfpga_stepmotor_timer;
	int select;			// select info
	int stepping_mode;		// motor stepping multiple (should be one of 1/2/4/8/16/32)
	struct ramp_info *rampinfo;	// ramp table of supported speeds and speed-shifts
	int pullin_speed;
	struct fpga_stepmotor_controller_ctrl controller_ctrl;
};


extern int fpga_stepmotor_controller_install(struct steppermotor *motor);
extern int fpga_stepmotor_controller_drvinit(void);

#endif


