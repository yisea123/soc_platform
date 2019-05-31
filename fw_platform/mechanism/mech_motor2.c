#include "respcode.h"
#include "mech_sensor2.h"
#include "mech_motor2.h"


static void step_motor_reset(struct motor_data *pmotor_data )
{
	steppermotor_reset(pmotor_data->motor_dev.psteppermotor); 
}
//===============================================================
static int32_t step_motor_start(mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data,
	struct motor_data *pmotor_data,unsigned char dir,  motor_mov_t  * pmotor_mov)		
{
	unsigned char i;
	int32_t ret=0;
	
	unsigned int sensor_mask;
	if ((pmotor_mov->speed_phase_num > STEPSPEED_PHASE_NUM) || (pmotor_mov->trigger_phase_num >2)) {
		return -RESN_MECH_ERR_IVALID_CMD;
	}
	
	pmotor_data->pmotor_mov = pmotor_mov;

	// get speed config and set
	pmotor_data->step_config.steps_to_run = 0;
	pmotor_data->step_config.dir = dir;
	pmotor_data->step_config.num_speed = pmotor_data->pmotor_mov->speed_phase_num;
	pmotor_data->step_config.speedinfo = pmotor_data->speedinfo;
	pmotor_data->moving_status = MOTOR_MOVE_STATUS_INUSE; 

	pmotor_data->stoping_status = 0;
	pmotor_data->err_status = 0;

	for (i=0; i<pmotor_mov->speed_phase_num; i++) {
		pmotor_data->speedinfo[i].speed = pmotor_data->pmotor_mov->motor_speed_phase[i].speed;
		pmotor_data->speedinfo[i].steps = pmotor_data->pmotor_mov->motor_speed_phase[i].speed_steps;
		pmotor_data->step_config.steps_to_run += pmotor_data->pmotor_mov->motor_speed_phase[i].speed_steps;
		if (i<pmotor_mov->speed_phase_num-1){
			pmotor_data->speedinfo[i].nextspeed = &pmotor_data->speedinfo[i+1];
		}
		else
			pmotor_data->speedinfo[i].nextspeed = NULL;
	}

	ret=steppermotor_set_config(pmotor_data->motor_dev.psteppermotor, &(pmotor_data->step_config));
	if(ret)
	{
		//printk(KERN_ERR "step_motor_start:steppermotor_set_config error!\n");
		ret = -RESN_MECH_ERR_MOTOR_MOVE_SET_CONFIG_ERR;
		return ret;
	}

	pmotor_data->moving_status = MOTOR_MOVE_STATUS_RUNNING; 
	ret=steppermotor_start(pmotor_data->motor_dev.psteppermotor);
	if(ret)
	{
		pmotor_data->moving_status = MOTOR_MOVE_STATUS_INUSE; 
		//printk(KERN_ERR "step_motor_start:steppermotor_start error!\n");
		return ret;
	}

	//printk(KERN_DEBUG "step_motor_start.................over!\n");
	return ret;
}

static unsigned char bstep_motor_stoped(struct motor_data *pmotor_data)
{
	int32_t status = steppermotor_status(pmotor_data->motor_dev.psteppermotor);
	//if (IS_ERR_VALUE(status)) {
	if(status<0)	{//needfill
		return 0;
	}
	//printk(KERN_DEBUG "bstep_motor_stoped:status=%x\n", status);
	if (steppermotor_is_running(status))
		return 0;
	return 1;
}

static int32_t step_motor_wait_stop(struct motor_data *pmotor_data)
{
	uint32_t timeout_ret;
	int32_t ret=0;

	//printk(KERN_DEBUG "step_motor_wait_stop:stoping_status=%lx %lx %d", pmotor_data->stoping_status, (pmotor_data->stoping_status & MOTOR_STOP_MASK), ((pmotor_data->stoping_status & MOTOR_STOP_MASK) ==MOTOR_STOP_BY_ABNORMAL));
	if ((pmotor_data->stoping_status & MOTOR_STOP_MASK) ==MOTOR_STOP_BY_ABNORMAL) {
		ret = pmotor_data->err_status;
		//printk(KERN_DEBUG "ret=%x\n",ret);
		
	}
	else
		ret = 0;
	
	pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
	return ret;
}
//-------------------------------------------
static int32_t brushdc_motor_start(mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data,
		struct motor_data *pmotor_data, unsigned char dir,  motor_mov_t  * pmotor_mov)
{
	unsigned char i;
	int32_t ret=0;

	//printk(KERN_DEBUG "brushdc_motor_start.................speed_phase_num=%d\n", pmotor_mov->speed_phase_num);

	pmotor_data->pmotor_mov = pmotor_mov;

	pmotor_data->dc_config.dir = dir;
	pmotor_data->dc_config.time_to_run = pmotor_mov->motor_speed_phase->speed_steps;
	//printk(KERN_DEBUG "brushdc_motor_start:dir=%d \n", pmotor_data->dc_config.dir);
	//printk(KERN_DEBUG "pdcmotor=%x\n", (int)pmotor_data->motor_dev.pdcmotor);
	ret=dcmotor_set_config(pmotor_data->motor_dev.pdcmotor, &(pmotor_data->dc_config));
	if(ret)
	{
		//printk(KERN_ERR "brushdc_motor_start:dcmotor_set_config error!\n");
		ret = -RESN_MECH_ERR_MOTOR_MOVE_SET_CONFIG_ERR;
		return ret;
	}

	pmotor_data->moving_status = MOTOR_MOVE_STATUS_INUSE; 
	pmotor_data->stoping_status = 0;
	pmotor_data->err_status = 0;
    
	//printk(KERN_DEBUG "brushdc_motor_start:triger_num=%d", pmotor_data->pmotor_mov->trigger_phase_num);
 	ret=dcmotor_start(pmotor_data->motor_dev.pdcmotor);
	if(ret)
	{
		//printk(KERN_ERR "brushdc_motor_start:dcmotor_start error!\n");
		ret = -RESN_MECH_ERR_MOTOR_MOVE_START_ERR;
		return ret;
	}
	pmotor_data->moving_status = MOTOR_MOVE_STATUS_RUNNING; 
	//printk(KERN_INFO "brushdc_motor_start.................over!\n");
	return ret;
}

void brushdc_motor_stop(struct motor_data *pmotor_data )
{
	//printk(KERN_DEBUG "brushdc_motor_stop\n");
	dcmotor_stop(pmotor_data->motor_dev.pdcmotor); 
}

static unsigned char bbrushdc_motor_stoped(struct motor_data *pmotor_data)
{
	int status = dcmotor_status(pmotor_data->motor_dev.pdcmotor);

	//printk(KERN_DEBUG "bbrushdc_motor_stoped\n");
	if(dcmotor_is_running(status))
		return 0;
	return 1;
}

static int32_t brushdc_motor_wait_stop(struct motor_data *pmotor_data)
{
	int32_t timeout_ret;
	unsigned long time;
	int32_t ret=0;


	if (pmotor_data->pmotor_mov->motor_trigger_phase[0].sen_mask)
		time = pmotor_data->pmotor_mov->motor_trigger_phase[0].to_trigger_steps*3/2;
	else
		time = pmotor_data->pmotor_mov->motor_trigger_phase[0].to_trigger_steps;

	if (pmotor_data->moving_status == MOTOR_MOVE_STATUS_RUNNING) 
	{
		
	}

	pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
	//printk(KERN_DEBUG "brushdc_motor_wait_stop4\n");
	return ret;
}



/*----------------------------------------------------------------------- 
 
-----------------------------------------------------------------------*/
int32_t motor_move_init(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask)
{
	int ret = 0;
	
	if (ret = bmotor_stoped(punit_motor_data, motor_mask)) {
		return 0;
	}
	else
		return -RESN_MECH_ERR_MOTOR_BUSY;
}

/*
   

	
*/
int32_t motor_start(mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data,
	unsigned short motor_mask, unsigned char dir, motor_mov_t  *pmotor_mov)
{
	struct motor_data *pmotor_data;
	unsigned char i;

	//printk(KERN_DEBUG "motor_start.................\n");
    
	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}
    
    	
	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:		
		return step_motor_start(punit_motor_data, punit_sensor_data, pmotor_data, dir, pmotor_mov);
	
	case BRUSH_DCMOTOR:
		return brushdc_motor_start(punit_motor_data, punit_sensor_data, pmotor_data, dir, pmotor_mov);
	default:
		break;
	}
	//printk(KERN_DEBUG  "motor_start.................over!\n");
	return 0;
}

//----------------------stop motor moving and wait until stopped----------------------
int32_t motor_stop(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask)
{
	struct motor_data *pmotor_data;
	unsigned char i;
	int32_t ret=0;

	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}
    	
	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:
		if(bstep_motor_stoped(pmotor_data))
			break;
		pmotor_data->moving_status |= MOTOR_STOP_BY_SOFT_START; 
		steppermotor_stop(pmotor_data->motor_dev.psteppermotor);
		break;
	case BRUSH_DCMOTOR:
		if(bbrushdc_motor_stoped(pmotor_data))
			break;
		pmotor_data->moving_status |= MOTOR_STOP_BY_SOFT_START;
		brushdc_motor_stop(pmotor_data);
		break;
	default:
		break;
	}
	return ret;
}

int32_t bmotor_stoped(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask)
{
	struct motor_data *pmotor_data;
	unsigned char i;

	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}

	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:
		return bstep_motor_stoped(pmotor_data);
	case BRUSH_DCMOTOR:
		return bbrushdc_motor_stoped(pmotor_data);
	default:
		break;
	}
	return 0;
}

int32_t motor_continue(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask, unsigned long steps)
{
	struct motor_data *pmotor_data;
	unsigned char i;

	//printf("motor_continue1\n\r");
	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}

	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:
		//printf("motor_continue2\n\r");
		return steppermotor_set_running_steps(pmotor_data->motor_dev.psteppermotor, steps);
	#if 0		
	case BRUSH_DCMOTOR:
		return bbrushdc_motor_stoped(pmotor_data);
	#endif
	default:
		break;
	}
	return 0;
}


int32_t motor_wait_stop(mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data, unsigned short motor_mask)
{
	struct motor_data *pmotor_data;
	unsigned char i;
	int32_t ret=0;

	//printk(KERN_DEBUG "motor_wait_stop\n");

	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}
    
	//printk(KERN_DEBUG "motor_type=%x \n", pmotor_data->motor_type);

	
	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:
		ret = step_motor_wait_stop(pmotor_data);
		break;
	case BRUSH_DCMOTOR:
		ret = brushdc_motor_wait_stop(pmotor_data);
		break;
	default:
		break;
	}
	
	//printk(KERN_DEBUG "motor_wait_stop over!\n");
	return ret;
}

//----------------------motor_lock----------------------
int32_t motor_lock(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask)
{
	struct motor_data *pmotor_data;
	unsigned char i;
	int32_t ret=0;

	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}
    	
	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:
		steppermotor_lock(pmotor_data->motor_dev.psteppermotor); 
		break;
	default:
		break;
	}
	return ret;
}

//----------------------motor_unlock----------------------
int32_t motor_unlock(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask)
{
	struct motor_data *pmotor_data;
	unsigned char i;
	int32_t ret=0;

	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}
    	
	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:
		steppermotor_unlock(pmotor_data->motor_dev.psteppermotor); 
		break;
	default:
		break;
	}
	return ret;
}

//----------------------motor_get_running_steps----------------------
int32_t motor_get_running_steps(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask, int *psteps)
{
	struct motor_data *pmotor_data;
	unsigned char i;
	int32_t ret=0;

	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}
    	
	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:
		ret = steppermotor_get_running_steps(pmotor_data->motor_dev.psteppermotor); 
		#if 0	//needfill
		if (!IS_ERR_VALUE(ret)) 
		#else
		if(ret>0)
		#endif
		{
			*psteps = ret;
			ret = 0;
		}
		break;
	default:
		*psteps = 0;
		break;
	}
	return ret;
}

