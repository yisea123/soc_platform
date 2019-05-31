#include "mech_unit2.h"
#include "FreeRTOS.h"
#include "respcode.h"

//----------------------get the status of sensor(detected or undetected)----------------------
int32_t mechunit_get_sensor_status(struct mechanism_uint_data *punit_data,unsigned int sensor_masks, unsigned int *pstatus)
{
	unsigned char j, sensor_num;
	unsigned int mask, status=0;
	unsigned int val;
	int32_t ret=0;
	mechanism_uint_sensor_data_t *punit_sensor_data = &(punit_data->unit_sensor_data);
	struct sensor_data *psen_data;
	unsigned int tmp_masks=sensor_masks;
	
	sensor_num = punit_sensor_data->sensor_num;
	for (j = 0; j < sensor_num; j++) {
		psen_data = &(punit_sensor_data->sensor[j]);
		mask = psen_data->sen_mask;
		if(tmp_masks & mask){
			tmp_masks &= ~mask;
			
			if(psen_data->sensor_type==SENSOR_PHOTO_TYPE)
			{
				ret = photosensor_status(psen_data->sen_dev.pphotosensor, &val);
				if (ret < 0) {
					return ret;
				}
			}
			else if(psen_data->sensor_type==SENSOR_MICROSWITCH)
			{
				ret = simplesensor_status(psen_data->sen_dev.psimplesensor, &val);
				if (ret < 0) {
					return ret;
				}
			}
			if (val) {
				status |= mask;
			}
		}
		if(!tmp_masks)
			break;
	}
	*pstatus = status;
	return ret;
}

//----------------------get the output value of sensor----------------------	
int32_t mechunit_get_sensors_rawinput(struct mechanism_uint_data *punit_data, mech_unit_sen_raw_input_t * psen_raw_input)
{
	int32_t ret=0, i, j;
	mechanism_uint_sensor_data_t *punit_sensor_data = &(punit_data->unit_sensor_data);
	struct sensor_data *psen_data;
	sen_raw_input_t	*sen_raw_input;
	
	psen_raw_input->sen_num = punit_sensor_data->sensor_num;
	for (i=0; i < psen_raw_input->sen_num; i++) {
		sen_raw_input = &psen_raw_input->sen_raw_input[i];
		sen_raw_input->sen_mask = punit_sensor_data->sensor[i].sen_mask;
		sen_raw_input->raw_input_value[0] = 0;
	}

	//printf("mechunit_get_sensors_rawinput1:\n\r");
	for (j=1; j<=SENSOR_RAW_GET_NUM; j++) {
		//printf("round%d:\n\r", j);
		for (i = 0; i < psen_raw_input->sen_num; i++) {
			psen_data = &(punit_sensor_data->sensor[i]);
			sen_raw_input = &psen_raw_input->sen_raw_input[i];
			//printf("No.%d ", i);
			if(psen_data->sensor_type==SENSOR_PHOTO_TYPE)
			{				
				ret = photosensor_read_input(psen_data->sen_dev.pphotosensor, &(sen_raw_input->raw_input_value[j]));
				sen_raw_input->raw_input_value[0] += sen_raw_input->raw_input_value[j];
				if (ret) 
				{
					goto exit;
				}
			}
			else if(psen_data->sensor_type==SENSOR_MICROSWITCH)
			{
				ret =simplesensor_read_input(psen_data->sen_dev.psimplesensor, &(sen_raw_input->raw_input_value[j]));
				sen_raw_input->raw_input_value[0] += sen_raw_input->raw_input_value[j];
				if (ret) 
				{
					goto exit;
				}
			}
			//printf("val%x=%x\n\r", j, sen_raw_input->raw_input_value[j]);
		}
	}

	//printf("mechunit_get_sensors_rawinput2:\n\r");
	for (i=0; i < psen_raw_input->sen_num; i++) {
		psen_data = &(punit_sensor_data->sensor[i]);
		psen_raw_input->sen_raw_input[i].raw_input_value[0] /= SENSOR_RAW_GET_NUM;
		//printf("No.%d val0=%x\n\r", i, psen_raw_input->sen_raw_input[i].raw_input_value[0]);
	}
	
exit:         
	return ret;

}

#if 0	
int mechunit_set_sensor_config(struct mechanism_uint_data *punit_data, mech_unit_sen_config_t *pmech_unit_sen_config)
{
	unsigned char i;
	int32_t ret=0;

	for (i=0; i < pmech_unit_sen_config->sen_num; i++) {
		ret = sensor_set_config(&(punit_data->unit_sensor_data), pmech_unit_sen_config, &pmech_unit_sen_config->sen_config[i]);
		if (ret) {
			return ret;
		}
	}
	return 0;
}
#else
//----------------------set sensor config----------------------
int mechunit_set_sensor_config(struct mechanism_uint_data *punit_data, mech_unit_sen_config_t *pmech_unit_sen_config)
{
	struct sensor_data *psen_data;
	struct photosensor_config config;
	uint8_t i, sen_num;
	int32_t ret=0;
	sen_config_t *p_sen_config;
	mechanism_uint_sensor_data_t *punit_sensor_data;
	
	punit_sensor_data = &(punit_data->unit_sensor_data);
	sen_num = pmech_unit_sen_config->sen_num;
	for (i=0; i< sen_num; i++) {
		p_sen_config = &pmech_unit_sen_config->sen_config[i];
		psen_data = &((punit_sensor_data)->sensor[i]);

		if(psen_data->sensor_type==SENSOR_PHOTO_TYPE)
		{
			ret = photosensor_get_config(psen_data->sen_dev.pphotosensor, &config);
			if (ret) 
				break;
	     
			config.compare_threshold = p_sen_config->pps_ref_value; 
			config.led_brightness = p_sen_config->pps_drv_value; 
			ret = photosensor_set_config(psen_data->sen_dev.pphotosensor, &config);

			if (ret) 
				break;
		}
	}
	return 0;
}
#endif

//----------------------get feature of sensor----------------------
int mechunit_get_sensor_feature(struct mechanism_uint_data *punit_data, mech_unit_sen_feature_t *pmech_unit_sen_feature)
{
	unsigned char i;
	int32_t ret=0;

	mechanism_uint_sensor_data_t *punit_sensor_data = &(punit_data->unit_sensor_data);
	struct sensor_data *psen_data;
	struct photosensor_feature feature;
	sen_feature_t *p_ppsfeature;

	pmech_unit_sen_feature->sen_num = punit_sensor_data->sensor_num;
	for (i=0; i < pmech_unit_sen_feature->sen_num; i++) {
		psen_data = &(punit_sensor_data->sensor[i]);
		p_ppsfeature = &(pmech_unit_sen_feature->sen_feature[i]);
	
		p_ppsfeature->sen_mask = psen_data->sen_mask;
		switch(psen_data->sensor_type)
		{
		case SENSOR_PHOTO_TYPE:
			ret = photosensor_get_feature(psen_data->sen_dev.pphotosensor, &feature);
			if(!ret)
			{
				p_ppsfeature->led_brightness_max = feature.led_brightness_max;
				p_ppsfeature->raw_input_max = feature.raw_input_max;
				p_ppsfeature->input_scale_mv = feature.input_scale_mv; 
				p_ppsfeature->calibrate_mode = feature.calibrate_mode;
			}
			break;
		case SENSOR_MICROSWITCH:
			break;
		default:
			break;
		}
	}
	return ret;
	

}

int mechunit_get_motor_feature(struct mechanism_uint_data *punit_data, mech_unit_motor_feature_t *pmech_unit_motor_feature)
{
	unsigned char i;
	int32_t ret=0;
	mechanism_uint_motor_data_t *punit_motor_data = &(punit_data->unit_motor_data);
	motor_feature_t *p_motorfeature;
	struct motor_data *pmotor_data;
	
	pmech_unit_motor_feature->motor_num = punit_data->unit_motor_data.motor_num; 
	for (i=0; i < pmech_unit_motor_feature->motor_num; i++) {
		p_motorfeature = &pmech_unit_motor_feature->motor_feature[i];
		
		pmech_unit_motor_feature->motor_feature[i].motor_mask = punit_data->unit_motor_data.motor[i].motor_mask; 
		pmotor_data = &((punit_motor_data)->motor[i]);
		
		if (pmotor_data->motor_type==STEPPERMOTOR) {
			const struct steppermotor_feature *feature;
		
			feature = steppermotor_get_feature(pmotor_data->motor_dev.psteppermotor);
		
			if(feature>0)
			{
				p_motorfeature->step_max = feature->max_steps;
				p_motorfeature->pullin_speed = feature->pullin_speed;
				p_motorfeature->num_speed = feature->num_speed;
				for (i=0; i<p_motorfeature->num_speed; i++) {
					p_motorfeature->speeds[i].accel_steps = feature->speeds[i].accel_steps;
					p_motorfeature->speeds[i].decel_steps = feature->speeds[i].decel_steps;
					p_motorfeature->speeds[i].speed = feature->speeds[i].speed; 
				}                    
			}

		}
		else
			p_motorfeature->step_max = -1;
	}
	return ret;
}


int32_t mechunit_get_motor_running_steps(struct mechanism_uint_data *punit_data, unsigned short motor_mask, int *psteps)
{
	return motor_get_running_steps(&(punit_data->unit_motor_data), motor_mask, psteps);
}

//----------------------------------------------------------
int32_t 	mechunit_init(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control)
{
	int32_t ret=0, i;
	uint8_t motor_num;
	struct motor_data *pmotor_data;
	mechanism_uint_motor_data_t *punit_motor_data =&mech_dev->mech_unit_data.unit_motor_data;

	motor_num = punit_motor_data->motor_num;
	for (i=0; i < punit_motor_data->motor_num; i++) {
		pmotor_data = &((punit_motor_data)->motor[i]);
		pmotor_data->moving_status = MOTOR_MOVE_STATUS_INIT;
		pmotor_data->stoping_status = 0;
	
	}

	return ret;
}

uint32_t mechunit_event_set(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control)
{
	mech_event_t *pmech_event = (mech_event_t *)mech_control->buffer;
	sensor_event_t sensor_event=0;
	
	switch(pmech_event->event_flag)
	{
	case MOTOR_SEN_ARRIVE:
		sensor_event = SENSOR_EV_DETECTED;
		break;
	case MOTOR_SEN_LEAVE:
		sensor_event = SENSOR_EV_UNDETECTED;
		break;
	case (MOTOR_SEN_ARRIVE|MOTOR_SEN_LEAVE):
		sensor_event = SENSOR_EV_DETECTED|SENSOR_EV_UNDETECTED;
		break;
	default:
		break;
	}			

	sensor_set_event(&mech_dev->mech_unit_data.unit_sensor_data, pmech_event->sen_mask, sensor_event, pmech_event->event_handle);	
}

uint32_t mechunit_event_unset(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control)
{
	mech_event_t *pmech_event = (mech_event_t *)mech_control->buffer;
	sensor_event_t sensor_event;
	
	switch(pmech_event->event_flag)
	{
	case MOTOR_SEN_ARRIVE:
		sensor_event = SENSOR_EV_DETECTED;
		break;
	case MOTOR_SEN_LEAVE:
		sensor_event = SENSOR_EV_UNDETECTED;
		break;
	case (MOTOR_SEN_ARRIVE|MOTOR_SEN_LEAVE):
		sensor_event = SENSOR_EV_DETECTED|SENSOR_EV_UNDETECTED;
		break;
	default:
		return 0;
	}			

	sensor_unset_event(&mech_dev->mech_unit_data.unit_sensor_data, pmech_event->sen_mask, sensor_event);	
	return 0;
}

int32_t	mechunit_motor_move(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control)
{
	int32_t ret;
	mechunit_motor_mov_t  *pmech_motor_move;

	pmech_motor_move = (mechunit_motor_mov_t *)mech_control->buffer;

	ret = motor_move_init(&mech_dev->mech_unit_data.unit_motor_data, pmech_motor_move->motor_mask);
	if (ret) 
		return ret;
	
	ret = motor_start(&mech_dev->mech_unit_data.unit_motor_data, &mech_dev->mech_unit_data.unit_sensor_data,
		pmech_motor_move->motor_mask, pmech_motor_move->dir, pmech_motor_move->pmotor_mov);
	
	return ret;
}

int32_t	mechunit_motor_move_blocked(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control)
{
	int32_t ret;
	mechunit_motor_mov_t  *pmech_motor_move;

	pmech_motor_move = (mechunit_motor_mov_t *)mech_control->buffer;

	ret = motor_move_init(&mech_dev->mech_unit_data.unit_motor_data, pmech_motor_move->motor_mask);
	if (ret) 
		goto end_mech_motor_move;
	
	ret = motor_start(&mech_dev->mech_unit_data.unit_motor_data, &mech_dev->mech_unit_data.unit_sensor_data,
		pmech_motor_move->motor_mask, pmech_motor_move->dir, pmech_motor_move->pmotor_mov);
	if (ret) 
	{
		goto end_mech_motor_move;
	}
	
	ret = motor_wait_stop(&mech_dev->mech_unit_data.unit_motor_data, 
		&mech_dev->mech_unit_data.unit_sensor_data,
		pmech_motor_move->motor_mask); 
	
end_mech_motor_move:
	return ret;
}

int32_t	mechunit_stop(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control)
{
	unsigned short motor_mask = (unsigned short)pmech_control->buffer;

	motor_stop(&mech_dev->mech_unit_data.unit_motor_data, motor_mask); 
		
	return 0;
}

int32_t mechunit_waitstop(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control)
{
	while(1)
	{
		if(mechunit_is_stopped(mech_dev, pmech_control))
			return 0;
	}
	return -1;
}

int32_t	mechunit_is_stopped(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control)
{
	unsigned short motor_mask = (unsigned short)pmech_control->buffer;

	return bmotor_stoped(&mech_dev->mech_unit_data.unit_motor_data, motor_mask); 
}

int32_t	mechunit_continue(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control)
{
	unsigned short motor_mask = (unsigned short)pmech_control->buffer;
	
	return motor_continue(&mech_dev->mech_unit_data.unit_motor_data, motor_mask, pmech_control->steps);
}

int32_t	mechunit_sensor(struct mechanism_dev_t * mech_dev, mech_control_t *pmech_control)
{
	int32_t ret=0;

	switch(MECHCTRL_MODE(pmech_control))
	{
	case SENSOR_ON:
		ret = sensor_enable(&mech_dev->mech_unit_data.unit_sensor_data, MECHCTRL_BUFFER(pmech_control), 1); 
		break;
	case SENSOR_OFF:
		ret = sensor_enable(&mech_dev->mech_unit_data.unit_sensor_data, MECHCTRL_BUFFER(pmech_control), 0); 
		break;
	case SENSOR_SETCONFIG:
		ret = sensor_set_config(&mech_dev->mech_unit_data.unit_sensor_data, &(mech_dev->mech_unit_control.mech_unit_sen_config), (sen_config_t *)MECHCTRL_BUFFER(pmech_control)); 
		break;
	default:
		break;
	}
	return ret;
}

int32_t	mechunit_lock(struct mechanism_dev_t * mech_dev, mech_control_t *pmech_control)
{
	int32_t ret=0;
	unsigned short motor_mask = (unsigned short)pmech_control->buffer;

	ret = motor_lock(&mech_dev->mech_unit_data.unit_motor_data, motor_mask); 
	return ret;
}

int32_t	mechunit_unlock(struct mechanism_dev_t * mech_dev, mech_control_t *pmech_control)
{	
	int32_t ret=0;
	unsigned short motor_mask = (unsigned short)pmech_control->buffer;

	ret = motor_unlock(&mech_dev->mech_unit_data.unit_motor_data, motor_mask); 
	return ret;
}


int32_t	mechunit_motor_move_lock(struct mechanism_dev_t * mech_dev, mech_control_t *pmech_control)
{
	int32_t ret=0;
	unsigned short motor_mask = (unsigned short)pmech_control->buffer;
	
	ret = mechunit_motor_move(mech_dev, pmech_control);
	if (!ret) {
		ret = motor_lock(&mech_dev->mech_unit_data.unit_motor_data, motor_mask); 
	}
	return ret;
}



//-----------------------------------------------------------------

