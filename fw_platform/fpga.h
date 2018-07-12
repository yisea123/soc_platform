#ifndef __FPGA_H__
#define	__FPGA_H__


#define BIT(x) 	(1<<x)
#define REG_ADDR_SCAL	8

/* Section 1: FPGA control & information registers */
/* memory map (offset): */
#define FPGA_REG_CONTROL				
#define FPGA_REG_VERSION				


/* Section 2: FPGA interrupt registers */
/* memory map (offset to base address): */
#define FPGA_REG_INT_ENABLE				0x0000	
#define FPGA_REG_INT_CLEAR				0x0004	
#define FPGA_REG_INT_STATUS				0x0018


/* Section 3: position sensor (PWM) registers */
/* memory map (offset): */


/* Section 4: steppermotor control registers and speed profile table */
/* memory map (offset to unit base): */
#define FPGA_REG_MOTOR_CONTROL			0x0000
#define FPGA_REG_MOTOR_STATUS			0x0010
#define FPGA_REG_MOTOR_RUNNING_STEPS		0x000c
#define FPGA_REG_MOTOR_SCANSTART_STEPS		0x003c
#define FPGA_REG_MOTOR_ACCEL_STEPS		0x0040
#define FPGA_REG_MOTOR_CONST_STEPS		0x0044
#define FPGA_REG_MOTOR_DECEL_STEPS		0x0048


/* bits definition of FPGA_REG_MOTORx_CONTROL */
#define FPGA_REG_MOTOR_RUN			BIT(0)
#define FPGA_REG_MOTOR_DIRECTION		BIT(1)
#define FPGA_REG_MOTOR_STOP			(2 << 2)
#define FPGA_REG_MOTOR_EMERGENCY_BRAKE		(3 << 2)

#define FPGA_REG_MOTOR_CONTROL_MASK 		0xf

/* size of steppermotor speed ramp tables: */
#define FPGA_RAM_MOTOR_TABLE_RAMP_DEPTH		1024
#define FPGA_RAM_MOTOR_TABLE_COUNT_DEPTH	16
//
#define FPGA_RAM_MOTOR_TABLE_RAMP_SIZE		(4*FPGA_RAM_MOTOR_TABLE_RAMP_DEPTH)
#define FPGA_RAM_MOTOR_TABLE_COUNT_SIZE		(4*FPGA_RAM_MOTOR_TABLE_COUNT_DEPTH)

//
/* memory map (offset to unit base): */
#define FPGA_RAM_MOTOR_TABLE_RAMP		0x0000
#define FPGA_RAM_MOTOR_TABLE_COUNT		0x1e40


/* Section 5: DC-motor control registers */
/* memory map (offset to unit base): */
#define FPGA_REG_DCMOTOR_CONTROL		0x0000
#define FPGA_REG_DCMOTOR_STATUS			0x0004

/* bits definition of FPGA_REG_DCMOTOR_STATUS */
#define FPGA_REG_DCMOTOR_RUNNING		BIT(0)
#define FPGA_REG_DCMOTOR_STOPPED_BY_SENSOR	BIT(1)


/* Section 6: CIS control & information registers */
/* memory map (offset): */
#define FPGA_REG_CIS_CONTROL			0x0000
#define FPGA_REG_CIS_T1				0x0004
#define FPGA_REG_CIS_MAX_LIGHTON_TIME		0x0004
#define FPGA_REG_CIS_T1				0x0004
#define FPGA_REG_CIS_T1				0x0004
#define FPGA_REG_CIS_T1				0x0004
#define FPGA_REG_CIS_T1				0x0004

/* bits definition of FPGA_REG_CIS_CONTROL */
#define FPGA_REG_CIS_SCAN_ENABLE		BIT(0)
#define FPGA_REG_CIS_LEDS_ENABLE		BIT(1)
#define FPGA_REG_CIS_SCAN_TRIGGER_ENABLE	BIT(8)
#define FPGA_REG_CIS_SCANMODE_SIX_LIGHTS	(0x0 << 2)
#define FPGA_REG_CIS_SCANMODE_TEN_LIGHTS	(0x1 << 2)
#define FPGA_REG_CIS_SCANMODE_MASK		(0xf << 2)


/* Section 7: Image ADC control & information registers */
/* memory map (offset): */
#define FPGA_REG_IMGADC_CONTROL			0x0000

/* bits definition of FPGA_REG_IMGADC_CONTROL */
#define FPGA_REG_IMGADC1_ENABLE			BIT(0)
#define FPGA_REG_IMGADC2_ENABLE			BIT(1)
#define FPGA_REG_IMGADC3_ENABLE			BIT(2)
#define FPGA_REG_IMGADC4_ENABLE			BIT(3)
#define FPGA_REG_IMGADC5_ENABLE			BIT(4)

#endif
