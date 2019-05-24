#ifndef __FPGA_H__
#define	__FPGA_H__


#define BIT(x) 	(1<<(x))


/* Section 1: FPGA control & information registers */
/* memory map (offset): */
#define FPGA_REG_STATUS				0x0000
#define FPGA_REG_VERSION			0x0004
#define FPGA_REG_ENABLE 			0x0008
#define FPGA_REG_PRODUCT_TYPE 			0x000c

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
#define FPGA_REG_INT_SEL_TYPE6			BIT(5)
#define FPGA_REG_INT_SEL_TYPE7			BIT(6)
#define FPGA_REG_INT_SEL_TYPE8			BIT(7)
#define FPGA_REG_INT_SEL_ALL			BIT(15)

/* definition FPGA interrupt configurations. */
#define FPGA_IRQ_MODE_NONE			0
#define FPGA_IRQ_MODE_RISING_EDGE		1
#define FPGA_IRQ_MODE_FALLING_EDGE		2
#define FPGA_IRQ_MODE_BOTH_EDGES		3
#define FPGA_IRQ_MODE_MASK			3
#define FPGA_IRQ_MODE_BITWIDTH			2


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
#define FPGA_REG_CIS_DDR_PRESENT_WR_ADDR_L	0x1030
#define FPGA_REG_CIS_DDR_PRESENT_WR_ADDR_H	0x1034

/* bits definition of FPGA_REG_CIS_CONTROL */
#define FPGA_REG_CIS_SCAN_ENABLE		BIT(0)
#define FPGA_REG_CIS_LEDS_ENABLE		BIT(1)
#define FPGA_REG_CIS_SCANMODE_EN_RED		BIT(2)
#define FPGA_REG_CIS_SCANMODE_EN_GREEN		BIT(3)
#define FPGA_REG_CIS_SCANMODE_EN_BLUE		BIT(4)
#define FPGA_REG_CIS_SCANMODE_EN_IR		BIT(5)
#define FPGA_REG_CIS_SCANMODE_EN_GREYSCALE	BIT(6)
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


/* Section 8: simple PWM control registers */
/* memory map (offset): */
#define FPGA_REG_PWM_CONTROL			0x0000
#define FPGA_REG_PWM_PERIOD			0x0004
#define FPGA_REG_PWM_DUTY_1			0x0008

/* definition of PWM channel mask */
#define FPGA_REG_PWM_MASK(n)			BIT(n)


/* Section 9: simple GPIO registers */
/* memory map (offset): */
#define FPGA_REG_GPIO_INPUT			0x0000
#define FPGA_REG_GPIO_INT_ENABLE		0x0004
#define FPGA_REG_GPIO_INT_CLEAR			0x0008
#define FPGA_REG_GPIO_INT_STATUS		0x000c
#define FPGA_REG_GPIO_INT_MODE_1		0x0010
#define FPGA_REG_GPIO_INT_MODE_2		0x0014

/* definition of GPIO channel mask */
#define FPGA_REG_GPIO_MASK(n)			BIT(n)


/* Section 9: FPGA managed ADC registers */
/* memory map (offset): */
#define FPGA_REG_ADC_COMPARE_1			0x0000
#define FPGA_REG_ADC_COMPARE_2			0x0000
#define FPGA_REG_ADC_COMPARE_3			0x0000

#define FPGA_REG_ADC_THRESHOLD_1		0x0000

#define FPGA_REG_ADC_AVERAGE_1			0x0000

#define FPGA_REG_ADC_INT_ENABLE_1		0x0000
#define FPGA_REG_ADC_INT_ENABLE_2		0x0004
#define FPGA_REG_ADC_INT_ENABLE_3		0x0008
#define FPGA_REG_ADC_INT_CLEAR_1		0x000c
#define FPGA_REG_ADC_INT_CLEAR_2		0x0010
#define FPGA_REG_ADC_INT_CLEAR_3		0x0014
#define FPGA_REG_ADC_INT_STATUS_1		0x0018
#define FPGA_REG_ADC_INT_STATUS_2		0x001c
#define FPGA_REG_ADC_INT_STATUS_3		0x0020

#define FPGA_REG_ADC_INT_MODE_1			0x0024
#define FPGA_REG_ADC_INT_MODE_2			0x0028
#define FPGA_REG_ADC_INT_MODE_3			0x002c
#define FPGA_REG_ADC_INT_MODE_4			0x0030
#define FPGA_REG_ADC_INT_MODE_5			0x0034
#define FPGA_REG_ADC_INT_MODE_6			0x0038


/* definition of ADC channel mask */
#define FPGA_REG_ADC_MASK(n)			BIT(n)


#endif
