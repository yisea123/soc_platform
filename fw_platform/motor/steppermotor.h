#ifndef __STEPPERMOTOR_H__
#define __STEPPERMOTOR_H__

#include <stddef.h>
#include <stdint.h>
#include <motor.h>


/* definition of steppermotor status */
#define STEPPERMOTOR_RUNNING			(1 << 0)
#define STEPPERMOTOR_STOPPED_BY_TOTAL_STEPS	(1 << 1)
#define STEPPERMOTOR_FAULT			(1 << 15)
#define STEPPERMOTOR_STOPPED_MASK		(STEPPERMOTOR_STOPPED_BY_TOTAL_STEPS)

/* macros to test steppermotor status */
#define steppermotor_is_running(s)			((s) & STEPPERMOTOR_RUNNING)
#define steppermotor_is_stopped_by_total_steps(s)	((s) & STEPPERMOTOR_STOPPED_BY_TOTAL_STEPS)
#define steppermotor_is_fault(s)			((s) & STEPPERMOTOR_FAULT)

/* definition of steppermotor features */
#define STEPPERMOTOR_FIXED_SPEEDS			(1 << 0)
#define STEPPERMOTOR_SUPPORT_LOCK			(1 << 1)


/* macros to test steppermotor features */
#define steppermotor_is_fixed_speeds(f)			((f) & STEPPERMOTOR_FIXED_SPEEDS)
#define steppermotor_is_support_lock(f)			((f) & STEPPERMOTOR_SUPPORT_LOCK)


#define MAX_SPEED_NUMS		32
#define STOP_TABLE_DEEP		(256)
#define MOTO_RUN_STEP_MASK	0xffff

typedef enum{
	STEP_MODE_FULL=1,
	STEP_MODE_HALF=2,
	STEP_MODE_QUARTER=4,
	STEP_MODE_8MICRO=8,
	STEP_MODE_16MICRO=16
} step_mode_t;


/* detailed information of a speed */
struct speed_detail {
	int speed;				// speed (in SPS [full Steps per Second])
	int accel_steps;			// full steps to accelerate
	int decel_steps;			// full steps to decelerate
};



/* steppermotor feature information block */
struct steppermotor_feature {
	int feature_flag;				// feature flags
	int max_steps;					// maximum steps value (hardware or software limitation)
	int pullin_speed;				// pull-in speed (in SPS [full Steps per Second])
	int num_speed;					// number of speed supported
	struct speed_detail speeds[MAX_SPEED_NUMS];	// list of supported speeds
};


/* speed information block to define a speed section */
struct speed_info {
	int speed;				// speed in SPS (full Steps per Second)
	int steps;				// number of steps to run at current speed
	struct speed_info *nextspeed;		// pointer to next speed information block
};


/* steppermotor configuration block to define motion direction, total steps and speed profile information */
struct steppermotor_config {
	motion_dir dir;				// motor motion direction
	int steps_to_run;			// total number of steps to run
	int num_speed;				// number of speed info blocks
	struct speed_info *speedinfo;		// pointer to the first speed information block
};


/*
 * struct steppermotor
 *
 * One for each steppermotor device.
 */
struct steppermotor {
	const void *resource;			// pointer to the resource block of a steppermotor instance
	int (*const install)(struct steppermotor *motor);	// pointer to the device installation function 
	struct steppermotor_feature feature;
	int status;
	struct steppermotor_config config;	// copy of current motor motion configuration
	const struct steppermotor_ops *ops;
	void (*callback)(struct steppermotor *motor, struct callback_data *data);	// callback handling steppermotor events after a motor interrupt
	struct callback_data callbackdata;
};


/*
 * struct steppermotor_ops - steppermotor operations
 * @config: set configure of this steppermotor
 * @status: get status of this steppermotor
 * @start: start running this steppermotor
 * @stop: stop running this steppermotor gracefully 
 * @emergencybrake: stop running this steppermotor in emergency 
 * @lock: (optional) lock this steppermotor
 * @unlock: (optional) unlock this steppermotor
 * @get_running_steps: get running steps of this steppermotor
 */ 
struct steppermotor_ops {
	int	(*config)(struct steppermotor *motor, const struct steppermotor_config *config);
	int	(*status)(struct steppermotor *motor);

	int	(*start)(struct steppermotor *motor);
	void	(*reset)(struct steppermotor *motor);
	int	(*lock)(struct steppermotor *motor);
	int	(*unlock)(struct steppermotor *motor);
	void	(*emergencybrake)(struct steppermotor *motor);
	void	(*stop)(struct steppermotor *motor);
	int	(*get_running_steps)(struct steppermotor *motor);
        
	int 	(*set_running_steps)(struct steppermotor *motor, int steps);
};


static inline const struct steppermotor_feature *steppermotor_get_feature(struct steppermotor *motor)
{
	return (!motor) ? NULL : &motor->feature;
}

/* declaration of steppermotor list */
extern struct steppermotor steppermotor_list[];
extern const int steppermotor_num;

/* upper-level function prototypes of steppermotor driver */
extern int steppermotor_install_devices(void);

extern struct steppermotor *steppermotor_get(int index);

extern int steppermotor_get_config(struct steppermotor *motor, struct steppermotor_config *config);
extern int steppermotor_set_config(struct steppermotor *motor, const struct steppermotor_config *config);

extern int steppermotor_set_callback(struct steppermotor *motor, void (*callback)(struct steppermotor *, struct callback_data *),
			      struct callback_data *data);
extern int steppermotor_status(struct steppermotor *motor);

extern int steppermotor_start(struct steppermotor *motor);
extern void steppermotor_reset(struct steppermotor *motor);
extern void steppermotor_stop(struct steppermotor *motor);
extern void steppermotor_emergencybrake(struct steppermotor *motor);
void steppermotor_lock(struct steppermotor *motor);
void steppermotor_unlock(struct steppermotor *motor);

int steppermotor_get_running_steps(struct steppermotor *motor);
int steppermotor_set_running_steps(struct steppermotor *motor, int steps);

#endif /* __STEPPERMOTOR_H__ */
