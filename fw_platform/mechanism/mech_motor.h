#ifndef __MECH_MOTOR_H__
#define __MECH_MOTOR_H__

#include "steppermotor.h"
//#include "dcmotor.h"
#include "photosensor.h"
#include "mech_sensor.h"
#include "mechlib.h"
#include "fpga_stepmotor.h"
#include "motor.h"

//-------------------------------------------------------
#define STEPSPEED_PHASE_NUM 1

typedef  union{
	    //void (*dcmotor_callback)(struct dcmotor *, struct callback_data *);
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

//----------------------------------------------------------
struct motor_data{
	unsigned short motor_mask;
	//unsigned char motor_name[MECHUINT_NAME_LEN];
	union{
		struct steppermotor *psteppermotor;
		//struct dcmotor *pdcmotor;
	}motor_dev;
	unsigned char motor_type;
// about move config
    //for sensor triger
	#if 0
    //unsigned char triger_num;
    motion_condition_detect_t trigerinfo[STEPSPEED_PHASE_NUM];
    unsigned char phase_current_num;
    unsigned int last_sen_mask;		//sensors roll in this phase
    //unsigned char  last_motor_sen_flag;  //MOTOR_SEN_ARRIVE / MOTOR_SEN_LEAVE
	#endif
    //for stepmotor move
    struct steppermotor_config step_config;
    struct speed_info speedinfo[STEPSPEED_PHASE_NUM];
    //for dcmotor move
	#if 0
    struct dcmotor_config dc_config;
#endif
    motor_mov_t *pmotor_mov;

// completion
    struct completion motor_completion;
    unsigned int motor_comp_accout;
    struct completion motor_phase_completion;
    unsigned int motor_phase_accout;

//about moving status
    /*#define MOTOR_MOVE_STATUS_NORMAL    0x000
    #define MOTOR_WAIT_TRIGER_TIMEOUT   0x100
    #define MOTOR_WAIT_STOP_TIMEOUT     0x200*/
    unsigned char moving_status;
    unsigned long stoping_status;
    int err_status;
    motor_callback_t callback;
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
extern int32_t motor_wait_stop(mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data, unsigned short motor_mask);
extern int32_t motor_get_feature(mechanism_uint_motor_data_t *punit_motor_data, motor_feature_t *p_motorfeature);
extern int32_t motor_lock(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask);
extern int32_t motor_unlock(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask);
extern int32_t motor_get_running_steps(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask, int *psteps);
#endif

