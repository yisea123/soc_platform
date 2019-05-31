#ifndef __MECH_MOTOR2_H__
#define __MECH_MOTOR2_H__

#include "steppermotor.h"
#include "dcmotor.h"
#include "photosensor.h"
#include "mech_sensor2.h"
#include "mechlib2.h"
#include "fpga_stepmotor.h"
#include "motor.h"

//-------------------------------------------------------

#define STEPSPEED_PHASE_NUM 1
#if 0

typedef  union{
	    void (*dcmotor_callback)(struct dcmotor *, struct callback_data *);
	    void (*steppermotor_callback)(struct steppermotor *, struct callback_data *);
}motor_callback_t;

//----------------------------------------------------------
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

struct completion{
	uint8_t  done;
};

static inline void init_completion(struct completion *x)
{
	x->done = 0;
}

/*Return: 0 if timed out, and positive (at least 1) if completed.
*/
static inline unsigned long wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{
  	while((!x->done)&&timeout)
	{
		vTaskDelay(1/ portTICK_RATE_MS);
		if(timeout)
			timeout--;					
	}
	if(!x->done)
		return timeout;
	x->done--;
	return timeout?timeout:1;
}

static inline void complete(struct completion *x)
{
	x->done++;
}

#define UINT_MAX    (~0U)
static inline void complete_all(struct completion *x)
{
     	x->done += UINT_MAX/2;
}
#endif
//----------------------------------------------------------
struct motor_data{
	char motor_name[MECHUINT_NAME_LEN];
	unsigned short motor_mask;
	union{
		struct steppermotor *psteppermotor;
		struct dcmotor *pdcmotor;
	}motor_dev;
	unsigned char motor_type;
// about move config
    //for stepmotor move
    struct steppermotor_config step_config;
    struct speed_info speedinfo[STEPSPEED_PHASE_NUM];
    //for dcmotor move
    struct dcmotor_config dc_config;

    motor_mov_t *pmotor_mov;

//about moving status
    /*#define MOTOR_MOVE_STATUS_NORMAL    0x000
    #define MOTOR_WAIT_TRIGER_TIMEOUT   0x100
    #define MOTOR_WAIT_STOP_TIMEOUT     0x200*/
    unsigned char moving_status;
    unsigned long stoping_status;
    int err_status;
    int steps_to_stop;
};

typedef struct {
	unsigned char  motor_num;
	struct motor_data *motor;
}mechanism_uint_motor_data_t;

#define motor_get_data(punit_motor_data, motormask, pmotor_data, i) \
     for(i=0; i<(punit_motor_data)->motor_num; i++) \
        if((punit_motor_data)->motor[i].motor_mask== (motormask)) \
        {\
            pmotor_data = &((punit_motor_data)->motor[i]); \
            break;\
        } \


extern int32_t motor_move_init(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask);
extern int32_t motor_start(mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data, 
	unsigned short motor_mask, unsigned char dir, motor_mov_t  *pmotor_mov);
extern int32_t motor_stop(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask);
extern int32_t bmotor_stoped(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask);
extern int32_t motor_continue(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask, unsigned long steps);
extern int32_t motor_wait_stop(mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data, unsigned short motor_mask);
extern int32_t motor_get_feature(mechanism_uint_motor_data_t *punit_motor_data, motor_feature_t *p_motorfeature);
extern int32_t motor_lock(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask);
extern int32_t motor_unlock(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask);
extern int32_t motor_get_running_steps(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask, int *psteps);
#endif

