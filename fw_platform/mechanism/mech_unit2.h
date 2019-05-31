#ifndef __MECH_UNIT_H__
#define __MECH_UNIT_H__

#include "steppermotor.h"
//#include "../motor/dcmotor.h"
#include "photosensor.h"
//#include "../fpga_io.h"

#include "mech_motor2.h"
#include "mech_sensor2.h"
#include "mechlib2.h"
//--------------------------------------------------------
struct mechanism_uint_data{
	char mech_unit_name[MECHUINT_NAME_LEN];
	mechanism_uint_motor_data_t	unit_motor_data;
	char bmotor_filled;
	mechanism_uint_sensor_data_t 	unit_sensor_data;
	char bsensor_filled;
};

/*-----------------------------------------------------------------------
	struct mechanism_dev_t
 -----------------------------------------------------------------------*/
struct mechanism_dev_t {
	int driver_status;
	int mech_status;
	struct mechanism_uint_data mech_unit_data;
	mech_unit_control_t mech_unit_control;
	mechunit_drv_status_t mech_unit_drv_status;
};

#define	MECHCTRL_DIR(cptr)		(((mech_control_t *)cptr)->dir)
#define	MECHCTRL_MODE(cptr)	    	(((mech_control_t *)cptr)->mode)
#define MECHCTRL_SPEED(cptr)       	(((mech_control_t *)cptr)->time)
#define	MECHCTRL_STEPS(cptr)	    	(((mech_control_t *)cptr)->steps)
#define	MECHCTRL_BUFFER(cptr)	    	(((mech_control_t *)cptr)->buffer)

//--------------------------------------------------------
typedef struct {
	uint8_t event_flag;	//MOTOR_SEN_ARRIVE/MOTOR_SEN_LEAVE
	uint32_t sen_mask;
	void (*event_handle)(void);
}mech_event_t;

//--------------------------------------------------------

typedef unsigned char byte;
typedef unsigned short word;
typedef long address;

typedef	struct {		/* command class item		*/
    address		argptr;		/* argument pointer*/	
	//byte		*statusptr;	/* status byte pointer*/
} class_cmd;
 	
typedef	int	(*seqptr_t)(struct mechanism_dev_t *, class_cmd *);

extern int32_t 	mechunit_init(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control);
extern uint32_t mechunit_event_set(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control);
extern uint32_t mechunit_event_unset(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control);
extern int32_t mechunit_get_sensor_status(struct mechanism_uint_data *punit_data,unsigned int sensor_masks, unsigned int *pstatus);
extern int32_t mechunit_get_sensors_rawinput(struct mechanism_uint_data *punit_data, mech_unit_sen_raw_input_t * psen_raw_input);
//extern int32_t mechunit_set_sensor_config(struct mechanism_uint_data *punit_data, mech_unit_sen_config_t *pmech_unit_sen_config);
extern int32_t mechunit_get_sensor_feature(struct mechanism_uint_data *punit_data, mech_unit_sen_feature_t *pmech_unit_sen_feature);
extern int32_t mechunit_get_motor_feature(struct mechanism_uint_data *punit_data, mech_unit_motor_feature_t *pmech_unit_motor_feature);
extern int32_t mechunit_get_motor_running_steps(struct mechanism_uint_data *punit_data, unsigned short motor_mask, int *psteps);
extern int32_t	mechunit_stop(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control);
extern int32_t mechunit_waitstop(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control);
extern int32_t	mechunit_is_stopped(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control);
extern int32_t	mechunit_continue(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control);
extern int32_t	mechunit_motor_move(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control);
extern int32_t	mechunit_sensor(struct mechanism_dev_t * mech_dev, mech_control_t *pmech_control);

//-----mech_status--------------------------------------
#define	IDLE			0	/* devices states	*/ 
#define	BUSY			1
#define	RECOVERABLE		2
#define	UNRECOVERABLE		4
#define DEVICE_IN_ERROR	(RECOVERABLE | UNRECOVERABLE)

//------mechanism driver states-------------------------
#define	MECH_DRIVER_IDLE			(word)0		/*HL:驱动空闲*/
#define	MECH_DRIVER_OPENED			(word)1		/*HL:驱动打开*/
#define	MECH_DRIVER_WORKING			(word)2		/*HL:驱动工作*/
#define	MECH_DRIVER_DEFERRED 			(word)3		/*HL:驱动延迟*/
#define	MECH_DRIVER_CLOSING			(word)4		/*HL:驱动正在关闭*/
#define	MECH_DRIVER_TERMINATED			(word)5		/*HL:驱动结束*/

#define	MECH_DRIVER_WORKING_ASYNC		(word)6		/*HL:驱动异步方式工作*/

//-------------------------------
#endif
