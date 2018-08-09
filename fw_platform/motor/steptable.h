#ifndef __STEPTABLE_H__
#define __STEPTABLE_H__

#include <stdint.h>
#include "steppermotor.h"


/*******************************************************************************
 * data structures of lower-level steppermotor driver implemention	       *
 *******************************************************************************/

#define MAX_RAMP_LEN	1024

/* struct speed_ramp (ramptable of a speed) */
struct speed_ramp {
	struct motor_speedtable *accel_table;	// pointer to accelation speedtable
	struct motor_speedtable *decel_table;	// pointer to decelation speedtable
	uint32_t step_ticks;				// ticks per step at current speed  
};



/* struct ramp_info (speeds + speed-shifts)*/
struct ramp_info {
	int num_speed;						// number of speed supported
	struct speed_ramp speeds[MAX_SPEED_NUMS];		// speed ramp tables
};

typedef enum{
	DEC_RAMP_SPECIFIC,
	DEC_RAMP_REVERSED
}dec_ramp_attr_t;

/*
 * struct steppermotor speed table
 */
struct motor_speedtable {
	int stepping;
	uint32_t ramp_size;
	uint32_t start_speed;
	uint32_t object_speed;
	uint32_t *ramp_table;
	dec_ramp_attr_t dec_ramp_attr;
};


/*******************************************************************************
 * definitions of macro to lookup steppermotor speedtable		       *
 *******************************************************************************/
static inline int lookup_speedtable(struct steppermotor *motor, int speed)
{
	int i;
	for (i = 0; i < motor->feature.num_speed; i++)
		if (speed == motor->feature.speeds[i].speed)
			return i;
	return -1;
}


/*******************************************************************************
 * macro definitions of lower-level steppermotor driver implemention	       *
 *******************************************************************************/
int steppermotor_ramptable_convert(struct ramp_info *rampinfo, int clock);
int steppermotor_check_config(struct steppermotor *motor, const struct steppermotor_config *config);


#endif /* __STEPTABLES_H__ */
