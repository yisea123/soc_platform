#ifndef __MOTOR_H__
#define __MOTOR_H__

#define STEP_MOTOR_STEPS_STOP_FLAG		0x10000

typedef enum {
	STEPPERMOTOR,
	BRUSH_DCMOTOR,
	BRUSHLESS_DCMOTOR,
} motor_type;


typedef enum {
	MOTION_CLOCKWISE,
	MOTION_COUNTERCLOCKWISE
} motion_dir;


/*
 * struct callback_data
 */
struct callback_data {
	int data1;
	int data2;
};


#endif /* __MOTOR_H__ */

