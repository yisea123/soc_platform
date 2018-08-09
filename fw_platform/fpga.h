#ifndef __FPGA_H__
#define	__FPGA_H__


#define BIT(x) 	(1<<(x))


/* Section 1: FPGA control & information registers */
/* memory map (offset): */
#define FPGA_REG_STATUS				0x0000
#define FPGA_REG_VERSION			0x0004


/* Section 2: FPGA interrupt registers */
/* memory map (offset to base address): */
#define FPGA_REG_INT_ENABLE			0x0000	
#define FPGA_REG_INT_CLEAR			0x0004	
#define FPGA_REG_INT_STATUS			0x0018

/* bits definition of FPGA_REG_INT_xxx */
#define FPGA_REG_INT_SEL_TYPE1			BIT(0)
#define FPGA_REG_INT_SEL_TYPE2			BIT(1)
#define FPGA_REG_INT_SEL_TYPE3			BIT(2)
#define FPGA_REG_INT_SEL_TYPE4			BIT(3)
#define FPGA_REG_INT_SEL_TYPE5			BIT(4)
#define FPGA_REG_INT_SEL_ALL			BIT(15)


/* Section 3: position sensor (PWM) registers */
/* memory map (offset): */


/* Section 4: steppermotor control registers and speed profile table */
/* memory map (offset to unit base): */
#define FPGA_REG_MOTOR_CONTROL			0x0000
//#define FPGA_REG_MOTOR_STATUS			0x0010
#define FPGA_REG_MOTOR_RUNNING_STEPS		0x000c
//#define FPGA_REG_MOTOR_SCANSTART_STEPS		0x003c
#define FPGA_REG_MOTOR_ACCEL_STEPS		0x0040
#define FPGA_REG_MOTOR_CONST_STEPS		0x0044
#define FPGA_REG_MOTOR_DECEL_STEPS		0x0048


/* bits definition of FPGA_REG_MOTORx_CONTROL */
#define FPGA_REG_MOTOR_RUN			BIT(0)
#define FPGA_REG_MOTOR_DIRECTION		BIT(1)
#define FPGA_REG_MOTOR_STOP			(2 << 2)
#define FPGA_REG_MOTOR_EMERGENCY_BRAKE		(3 << 2)

#define FPGA_REG_MOTOR_CONTROL_MASK 		0xf

//
/* memory map (offset to unit base): */
#define FPGA_RAM_MOTOR_TABLE_RAMP		0x0000
//#define FPGA_RAM_MOTOR_TABLE_COUNT		0x1e40

/* bits definition of FPGA_RAM_MOTOR_TABLE_RAMP */
#define FPGA_RAM_MOTOR_TABLE_RAMP_ACCEL	0
#define FPGA_RAM_MOTOR_TABLE_RAMP_CONST1	(1<<14)
#define FPGA_RAM_MOTOR_TABLE_RAMP_DECEL	(3<<14)

/* Section 5: DC-motor control registers */
/* memory map (offset to unit base): */
#define FPGA_REG_DCMOTOR_CONTROL		0x0000
#define FPGA_REG_DCMOTOR_STATUS			0x0004

/* bits definition of FPGA_REG_DCMOTOR_STATUS */
#define FPGA_REG_DCMOTOR_RUNNING		BIT(0)
#define FPGA_REG_DCMOTOR_STOPPED_BY_SENSOR	BIT(1)


/* Section 6: scanning control & information registers */
/* memory map (offset): */
#define FPGA_REG_CIS_CONTROL			0x0000
#define FPGA_REG_CIS_T1				0x0004
#define FPGA_REG_CIS_MAX_LIGHTON_TIME		0x0008
#define FPGA_REG_CIS_DPI			0x1000
#define FPGA_REG_CIS_T_SI_H			0x1004
#define FPGA_REG_CIS_T_SI_L			0x1008
#define FPGA_REG_CIS_T_SI_L_PLUS		0x1024
#define FPGA_REG_CIS_SCANLINES			0x1028

/* bits definition of FPGA_REG_CIS_CONTROL */
#define FPGA_REG_CIS_SCAN_ENABLE		BIT(0)
#define FPGA_REG_CIS_LEDS_ENABLE		BIT(1)
#define FPGA_REG_CIS_SCANMODE_EN_RED		BIT(2)
#define FPGA_REG_CIS_SCANMODE_EN_GREEN		BIT(3)
#define FPGA_REG_CIS_SCANMODE_EN_BLUE		BIT(4)
#define FPGA_REG_CIS_SCANMODE_EN_GREYSCALE	BIT(5)
#define FPGA_REG_CIS_SCANMODE_EN_IR		BIT(6)
#define FPGA_REG_CIS_SCANMODE_EN_UV		BIT(7)
#define FPGA_REG_CIS_SCANMODE_EN_RGB		(FPGA_REG_CIS_SCANMODE_EN_RED | FPGA_REG_CIS_SCANMODE_EN_GREEN | FPGA_REG_CIS_SCANMODE_EN_BLUE)
#define FPGA_REG_CIS_SCANMODE_MASK		(0x3f << 2)


/* bits definition of FPGA_REG_CIS_DPI */
#define FPGA_REG_CIS_DPI_300			0x0
#define FPGA_REG_CIS_DPI_600			0x1
#define FPGA_REG_CIS_DPI_1200			0x2


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
