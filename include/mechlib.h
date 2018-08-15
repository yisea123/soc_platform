#ifndef __MECHLIB_H__
#define __MECHLIB_H__

#include <stdint.h>

/*export to application*/
#define MOTOR_ERR_OPTIMIZE	1

#define MECHUINT_NAME_LEN 20  // max. length of name defined in dts(device tree source). 
#define MAX_SPEED_NUMS		32
/*------------------------------------------------
    motor 
------------------------------------------------*/
#define mm_to_fullstep(distance, ratio)  ((distance)*ratio)


#define MOTOR_SEN_LEAVE	    0	// leave the sensors.
#define MOTOR_SEN_ARRIVE	1	// arrived the sensors.

//motor_trigger_stop_flag
#define MOTOR_TRIGGER_CONTINUE  0x00
#define MOTOR_TRIGGER_STOP      0x01
#define MOTOR_TRIGER_STOP_FLAG_MASK	0x01

//motor_trigger_condition_flag
#define MOTOR_TRIGGER_SENSOR_IMEDIAT    0x00
#define MOTOR_TRIGGER_SENSOR_END        0x02
#define MOTOR_TRIGER_SENSOR_FLAG_MASK	0x02

//motor_sensor_stop_flag
#define MOTOR_SENSOR_VALID_CONTINUE   	0x00
#define MOTOR_SENSOR_VALID_STOP       	0x04
#define MOTOR_SENSOR_STOP_FLAG_MASK	0x04

//motor_sensor_continue_mode
#define MOTOR_SENSOR_CONTINUE_DEFAULT	0x00	//continue util total steps end
#define MOTOR_SENSOR_CONTINUE_TRIGGER	0x08
#define MOTOR_SENSOR_CONTINUE_MODE_MASK	0x08

//motor_sensor_stop_mode
#define MOTOR_SENSOR_STOP_IMEDIATLY	0x00
#define MOTOR_SENSOR_STOP_AT_TRIGER_END	0x10
#define MOTOR_SENSOR_STOP_MODE_MASK	0x10

//motor_en_skew_steps
#define MOTOR_EN_SKEW_STEPS_DEFAULT	0x00
#define MOTOR_EN_SKEW_STEPS		0x20
#define MOTOR_EN_SKEW_STEPS_MASK	0x20

#if 1
/* photosensor trigger configuration block */
typedef struct  {
	int32_t motion_condition;				// motion_condition: 0 - from COVERED to UNCOVERED; 1 from UNCOVERED to COVERED;
	int32_t motion_detect;				// motion_detect:sensor_mask or position code
}motion_condition_detect_t;

typedef struct{
	unsigned long  to_trigger_steps; //max steps
	motion_condition_detect_t motion_condition_detect;
	uint32_t 	sen_mask;		//sensors roll in this phase	
	uint32_t 	sen_pos_index;		//sensors position index.If 0, avoid it.
	unsigned char  motor_sen_flag;  //MOTOR_SEN_ARRIVE / MOTOR_SEN_LEAVE
	struct{
		unsigned short motor_trigger_stop_flag:1;
		unsigned short motor_trigger_condition_flag:1;
		unsigned short motor_sensor_stop_flag:1;
		unsigned short  motor_sensor_continue_mode:1;
		unsigned short  motor_sensor_stop_mode:1;
		unsigned short motor_condition_type:1;	//0--sensor, 1--poscode		
	}motor_triger_flag;
}motor_trigger_phase_t;
#endif
//Stop at triger end ,regardless sensor
#define MOTORPHASE_STOP_AT_TRIGER_END	MOTOR_TRIGGER_STOP

//Judge sensor status at triger end. Stop when valid, continue when invalid. 
#define MOTORPHASE_SENSER_JUDGE_END_VALID_STOP_INVALID_TRIGER_CONTINUE \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_END|MOTOR_SENSOR_VALID_STOP|MOTOR_SENSOR_CONTINUE_TRIGGER)

#define MOTORPHASE_SENSER_JUDGE_END_VALID_STOP_INVALID_TOTAL_CONTINUE \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_END|MOTOR_SENSOR_VALID_STOP|MOTOR_SENSOR_CONTINUE_DEFAULT)

//Judge sensor status at triger end. Continue when valid， stop when invalid. 
#define MOTORPHASE_SENSER_JUDGE_END_VALID_TRIGER_CONTINUE_INVALID_STOP \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_END|MOTOR_SENSOR_VALID_CONTINUE|MOTOR_SENSOR_CONTINUE_TRIGGER)

#define MOTORPHASE_SENSER_JUDGE_END_VALID_TOTAL_CONTINUE_INVALID_STOP \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_END|MOTOR_SENSOR_VALID_CONTINUE|MOTOR_SENSOR_CONTINUE_DEFAULT)

//Judge sensor status immediately.Stop when valid，continue when invalid
#define MOTORPHASE_SENSER_IMEDIATLY_VALID_STOP_INVALID_TRIGER_CONTINUE \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_IMEDIAT|MOTOR_SENSOR_VALID_STOP|MOTOR_SENSOR_CONTINUE_TRIGGER)

#define MOTORPHASE_SENSER_IMEDIATLY_VALID_STOP_INVALID_TOTAL_CONTINUE \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_IMEDIAT|MOTOR_SENSOR_VALID_STOP|MOTOR_SENSOR_CONTINUE_DEFAULT)

//Judge sensor status immediately.Continue when valid，stop when invalid
#define MOTORPHASE_SENSER_IMEDIATLY_VALID_TRIGER_CONTINUE_INVALID_IMEDIATLY_STOP \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_IMEDIAT|MOTOR_SENSOR_VALID_CONTINUE|MOTOR_SENSOR_CONTINUE_TRIGGER|MOTOR_SENSOR_STOP_IMEDIATLY)

#define MOTORPHASE_SENSER_IMEDIATLY_VALID_TRIGER_CONTINUE_INVALID_UNTIL_TRIGER_STOP \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_IMEDIAT|MOTOR_SENSOR_VALID_CONTINUE|MOTOR_SENSOR_CONTINUE_TRIGGER|MOTOR_SENSOR_STOP_AT_TRIGER_END)

#define MOTORPHASE_SENSER_IMEDIATLY_VALID_TRIGER_CONTINUE_IMEDIATLY_SKEW_STOP_INVALID_UNTIL_TRIGER_STOP \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_IMEDIAT|MOTOR_SENSOR_VALID_CONTINUE|MOTOR_SENSOR_CONTINUE_TRIGGER|MOTOR_SENSOR_STOP_AT_TRIGER_END|MOTOR_EN_SKEW_STEPS)

#define MOTORPHASE_SENSER_IMEDIATLY_VALID_TOTAL_CONTINUE_INVALID_IMEDIATLY_STOP \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_IMEDIAT|MOTOR_SENSOR_VALID_CONTINUE|MOTOR_SENSOR_CONTINUE_DEFAULT|MOTOR_SENSOR_STOP_IMEDIATLY)

#define MOTORPHASE_SENSER_IMEDIATLY_VALID_TOTAL_CONTINUE_INVALID_UNTIL_TRIGER_STOP \
		(MOTOR_TRIGGER_CONTINUE|MOTOR_TRIGGER_SENSOR_IMEDIAT|MOTOR_SENSOR_VALID_CONTINUE|MOTOR_SENSOR_CONTINUE_DEFAULT|MOTOR_SENSOR_STOP_AT_TRIGER_END)

//--------------------------------------------------------
typedef struct{
	unsigned int speed;			//speed
	unsigned int speed_steps;		//max steps
}motor_speed_phase_t;	
	
typedef struct{
	unsigned int sensor_mask;
	unsigned char  attatched_trigger_phase_index;
	unsigned long  medialength_max;
	unsigned long  medialength_min;
}medialength_source_t;

typedef struct{
	//stepmotor move
	unsigned char speed_phase_num;
	motor_speed_phase_t  *motor_speed_phase;
	//sensor trigger
	unsigned char trigger_phase_num;
	motor_trigger_phase_t *motor_trigger_phase;
}motor_mov_t;

typedef struct{
	unsigned short motor_mask;
	unsigned char dir;
	motor_mov_t  *pmotor_mov;
}mechunit_motor_mov_t;
//------------------------------------------------
typedef struct{
	unsigned int sen_mask;
	unsigned long pps_ref_value;
	unsigned long pps_drv_value;
	int trigger_mode;				
	int trigger_enable;
	int calibrated;
}
sen_config_t;

typedef struct {
	unsigned char    sen_num;
	sen_config_t	*sen_config;
} 
mech_unit_sen_config_t;
//------------------------------------------------
//sen_feature is meaningful for analog sensors
typedef struct{
	unsigned int sen_mask;
	//unsigned char sen_name[MECHUINT_NAME_LEN];
	int led_brightness_max;         // maximum LED brightness level of photosensor
	unsigned long raw_input_max;        // maximum raw input value
	int input_scale_mv;         // scale to convert raw input value to voltage (mV)
	int coverd_mode;
	unsigned char calibrate_mode;       //0——only without paper， 1——only with paper
}
sen_feature_t;

typedef struct {
	unsigned char    	sen_num;
	sen_feature_t	*sen_feature;
} 
mech_unit_sen_feature_t;

#define senser_is_analog(psen_feature)	((((sen_feature_t *)psen_feature)->raw_input_max!=1)?1:0)
#define sensor_can_cal_withoutmedia(psen_feature)	((((sen_feature_t *)psen_feature)->calibrate_mode)?0:1)

//------------------------------------------------
/* detailed information of a speed */
typedef struct {
	int speed;				// speed (in SPS [full Steps per Second])
	int accel_steps;			// full steps to accelerate
	int decel_steps;			// full steps to decelerate
}stepmotor_speed_t;


/* information block to define a speed-shift */
typedef struct  {
	int speed1;				// speed 1 (in SPS)
	int speed2;				// speed 2 (in SPS)
	int steps;				// full steps to shift
}stepmotor_speed_shift_t;

//motor_feature is meaningful for stepermotor 
typedef struct{
	unsigned short motor_mask;
	//unsigned char motor_name[MECHUINT_NAME_LEN];
	int step_max;           // maximum LED brightness level of photosensor
	int pullin_speed;				// pull-in speed (in SPS [full Steps per Second])
	int num_speed;					// number of speed supported
	stepmotor_speed_t speeds[MAX_SPEED_NUMS];			// list of supported speeds
	int num_speedshift;				// number of speed-shift supported
	stepmotor_speed_shift_t speedshifts[MAX_SPEED_NUMS];	// list of supported speed-shifts 
}motor_feature_t;

typedef struct {
	unsigned char    	motor_num;
	motor_feature_t	*motor_feature;
} 
mech_unit_motor_feature_t;
//------------------------------------------------
#define SENSOR_RAW_GET_NUM	10

typedef struct{
	unsigned int sen_mask;
	unsigned long raw_input_value[SENSOR_RAW_GET_NUM+1];
}sen_raw_input_t;

typedef struct {
	unsigned char    sen_num;
	sen_raw_input_t	*sen_raw_input;
} 
mech_unit_sen_raw_input_t;

typedef struct {
	mech_unit_sen_feature_t 	mech_unit_sen_feature;
	mech_unit_sen_config_t 		mech_unit_sen_config;
	mech_unit_sen_raw_input_t 	mech_unit_sen_raw_input;
	mech_unit_motor_feature_t	mech_unit_motor_feature;
} 
mech_unit_control_t;

typedef struct {	
	unsigned char	mechdev_status;
	unsigned char	error_byte;     	/* error status descriptor	*/
	unsigned int  sen_status;
}
mechunit_drv_status_t;

typedef struct{
	unsigned short motor_mask;
	int motor_steps;
}mechunit_motor_steps_t;

typedef struct{
	//char mech_name[MECHUINT_NAME_LEN];
	unsigned char motor_num;
	unsigned char sensor_num;
}mechunit_feature_t;
//------------------------------------------------
//motor_data->moving_status bit0~bit7
#define MOTOR_MOVE_STATUS_INIT		0x01
#define MOTOR_MOVE_STATUS_INUSE		0x02	
#define MOTOR_MOVE_STATUS_RUNNING    	0x04
#define MOTOR_MOVE_STATUS_STOP		0x08
#define MOTOR_STOP_BY_SOFT_START	0x10

#define MOTOR_STOP_STATUS	(MOTOR_STOP_MASK|MOTOR_MOVE_STATUS_INIT)
//==========================================================
//------mechanism driver status code--------------------
//stop motor:bit8~bit11
#define MOTOR_STOP_UNIT_SHIFT		8
#define MOTOR_STOP_UNIT_MASK		(0xF<<MOTOR_STOP_UNIT_SHIFT)
#define MOTOR_STOP_UINT_TO_RES(x)	(x<<MOTOR_STOP_UNIT_SHIFT)

//stop status:bit4~bit7
#define MOTOR_STOP_STATUS_SHIFT		4
#define MOTOR_STOP_MASK			(0xF<<MOTOR_STOP_STATUS_SHIFT)	//(1111 0000b)
#define MOTOR_STOP_BY_SENSOR		(0x1<<MOTOR_STOP_STATUS_SHIFT)	//0x10:motor stoped by sensor status
#define MOTOR_STOP_BY_TRIGER		(0x2<<MOTOR_STOP_STATUS_SHIFT)	//0x20:motor stoped at the current triger end
#define MOTOR_STOP_BY_TOTAL		(0x3<<MOTOR_STOP_STATUS_SHIFT)	//0x30:motor stoped at total steps completed
#define MOTOR_STOP_BY_SOFT		(0x4<<MOTOR_STOP_STATUS_SHIFT)	//0x40:motor stoped by software
#define MOTOR_STOP_BY_SKEW        	(0x5<<MOTOR_STOP_STATUS_SHIFT)	//0x50:motor stoped by skew status
#define MOTOR_STOP_BY_ABNORMAL        	(0x6<<MOTOR_STOP_STATUS_SHIFT)	//0x60:abnormal stop


//supplementary info0:bit3
//motor_sen_flag(MOTOR_SEN_LEAVE/MOTOR_SEN_ARRIVE) when MOTOR_STOP_BY_SENSOR/MOTOR_STOP_BY_SKEW.
#define MOTOR_STOP_SEN_FLAG_SHIFT		3
#define MOTOR_STOP_SEN_FLAG_MASK		0x8

#define MOTOR_STOP_SEN_FLAG_TO_RES(x)	(x<<MOTOR_STOP_SEN_FLAG_SHIFT)
#define MOTOR_STOP_SEN_LEAVE		MOTOR_STOP_SEN_FLAG_TO_RES(MOTOR_SEN_LEAVE)		//0x0000
#define MOTOR_STOP_SEN_ARRIVE		MOTOR_STOP_SEN_FLAG_TO_RES(MOTOR_SEN_ARRIVE)		//0x0008

//supplementary info1:bit0~bit2
//sensor position when MOTOR_STOP_BY_SENSOR /MOTOR_STOP_BY_SKEW
#define MOTOR_STOP_SEN_POS_SHIFT		0
#define MOTOR_STOP_SEN_POS_MASK			0x7

#define MOTOR_STOP_SEN_POS_TO_RES(x)	(x<<MOTOR_STOP_SEN_POS_SHIFT)	
//--------------------
#define MOTOR_MOVE_STOPBYSENSOR(x)			(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_BY_SENSOR)
#define MOTOR_MOVE_STOPBYSENSOR_LEAVE(x, pos)		(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_SEN_POS_TO_RES(pos)+MOTOR_STOP_BY_SENSOR+MOTOR_STOP_SEN_LEAVE)  //0xe010
#define MOTOR_MOVE_STOPBYSENSOR_ARRIVE(x, pos)		(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_SEN_POS_TO_RES(pos)+MOTOR_STOP_BY_SENSOR+MOTOR_STOP_SEN_ARRIVE) //0xe018	

#define MOTOR_MOVE_STOPBYTRIGER(x)		(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_BY_TRIGER)	//0xe020
#define MOTOR_MOVE_STOPBYTOTAL(x)		(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_BY_TOTAL)	//0xe030
#define MOTOR_MOVE_STOPBYSOFT(x)		(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_BY_SOFT)	//0xe040
#define MOTOR_MOVE_STOPBYSKEW_LEAVE(x, pos)		(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_SEN_POS_TO_RES(pos)+MOTOR_STOP_BY_SKEW+MOTOR_STOP_SEN_LEAVE)	//0xe050
#define MOTOR_MOVE_STOPBYSKEW_ARRIVE(x, pos)		(RESN_OFFSET_MECH_DRIVER_MOV+MOTOR_STOP_UINT_TO_RES(x)+MOTOR_STOP_SEN_POS_TO_RES(pos)+MOTOR_STOP_BY_SKEW+MOTOR_STOP_SEN_ARRIVE)	//0xe058

/*------------------------------------------------

------------------------------------------------*/
//----------mode of mechunit_sensor() ----------
#define	SENSOR_ON			1
#define SENSOR_OFF                 2
#define SENSOR_SETCONFIG           3
//----------------------------------------


typedef struct {
	short	cmd;
	unsigned char	dir;
	unsigned short     mode;
	unsigned short speed;
	unsigned long	steps;
	long		buffer;
}mech_control_t;
#endif

