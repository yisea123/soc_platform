#ifndef __FPGA_DCMOTOR_CONTROLLER_H__
#define __FPGA_DCMOTOR_CONTROLLER_H__

#include "dcmotor.h"

#define FPGA_REG_DCMOTOR_CONTROLLER_CONTROL		0x0000
#define FPGA_REG_DCMOTOR_CONTROLLER_PERIOD		0x0004
#define FPGA_REG_DCMOTOR_CONTROLLER_DUTY		0x0008


/* bits definition of FPGA_REG_MOTORx_CONTROL */
#define FPGA_REG_DCMOTOR_CONTROLLER_RUN		BIT(0)
#define FPGA_REG_DCMOTOR_CONTROLLER_DIRECTION	BIT(1)
#define FPGA_REG_DCMOTOR_CONTROLLER_PWM		BIT(2)	//0-pwm enable;1-pwm disable


#define FPGA_DCMOTOR_CONTROLLER_DIRECTION(x)	(x<<1)

#define FPGA_REG_DCMOTOR_CONTROLLER_CONTROL_MASK 	0x3

/* definition of dcmotor controller ctrl status */
typedef enum{
	DCMOTOR_CONTROLLER_RUNNING_INIT,
	DCMOTOR_CONTROLLER_RUNNING,
	DCMOTOR_CONTROLLER_STOP_DELAY_INIT,
	DCMOTOR_CONTROLLER_STOP_DELAY,
	DCMOTOR_CONTROLLER_STOP
}dc_controller_ctrl_status_t;

struct fpga_dcmotor_controller_ctrl {
	int time_ms;		//ms
	int time_us;
	int time_ns;
	motion_dir dir;				
	dc_controller_ctrl_status_t ctrl_status;
};

struct fpga_dcmotor_controller_resource {
	void *mmio_base;	// registers base address
	int pclk_period;	// PWM instance system clock (PCLK) period in nanosecond (ns)
	int prescale;		// precale of PWM instance
	int period;		// PWM period in nanosecond (ns)
	int duty;		// %
	struct fpga_timer *pfpga_dcmotor_timer;
	struct fpga_dcmotor_controller_ctrl controller_ctrl;
};


extern int fpga_dcmotor_controller_install(struct dcmotor *motor);
extern int fpga_dcmotor_controller_drvinit(void);

#endif



