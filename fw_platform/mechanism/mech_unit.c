#include "mech_unit.h"
#include "FreeRTOS.h"
#include "respcode.h"

//----------------------get the status of sensor(detected or undetected)----------------------
int32_t mechunit_get_sensor_status(struct mechanism_uint_data *punit_data,unsigned int sensor_masks, unsigned int *pstatus)
{
	unsigned char j;
	unsigned int mask, status=0;
	unsigned int val;
	int32_t ret=0;

	#if 0
	for (j = 0; j < punit_data->unit_sensor_data.sensor_num; j++) {
		if(sensor_masks & punit_data->unit_sensor_data.sensor[j].sen_mask){
			mask = punit_data->unit_sensor_data.sensor[j].sen_mask;
			ret = sensor_get_appval(&(punit_data->unit_sensor_data), mask, &val);
			if (ret < 0) {
				return ret;
			}
			if (val) {
				status |= punit_data->unit_sensor_data.sensor[j].sen_mask;
			}
		}
	}
	*pstatus = status;
	return ret;
	#else
	mechanism_uint_sensor_data_t *punit_sensor_data = &(punit_data->unit_sensor_data);
	struct sensor_data *psen_data;
	
	for (j = 0; j < punit_sensor_data->sensor_num; j++) {
		if(sensor_masks & punit_sensor_data->sensor[j].sen_mask){
			mask = punit_sensor_data->sensor[j].sen_mask;
			psen_data = &(punit_sensor_data->sensor[j]);
			//ret = sensor_get_appval(&(punit_data->unit_sensor_data), mask, &val);
			ret =  photosensor_status(psen_data->sen_dev.pphotosensor, &val); 
			if (ret < 0) {
				return ret;
			}
			if (val) {
				status |= punit_data->unit_sensor_data.sensor[j].sen_mask;
			}
		}
	}
	*pstatus = status;

	if(j == punit_sensor_data->sensor_num)
		ret = -RESN_MECH_ERR_SENSOR_GETDATA;
	return ret;
	#endif
}

//----------------------get the output value of sensor----------------------	
int32_t mechunit_get_sensors_rawinput(struct mechanism_uint_data *punit_data, mech_unit_sen_raw_input_t * psen_raw_input)
{
	int32_t ret=0, i, j;

#if 0
	psen_raw_input->sen_num = punit_data->unit_sensor_data.sensor_num;
	for (i=0; i < psen_raw_input->sen_num; i++) {
		psen_raw_input->sen_raw_input[i].sen_mask = punit_data->unit_sensor_data.sensor[i].sen_mask;
		psen_raw_input->sen_raw_input[i].raw_input_value[0] = 0;
	}

	for (j=1; j<=SENSOR_RAW_GET_NUM; j++) {
		for (i = 0; i < psen_raw_input->sen_num; i++) {
			ret = sensor_get_val(&(punit_data->unit_sensor_data), 
						psen_raw_input->sen_raw_input[i].sen_mask, 
					     &(psen_raw_input->sen_raw_input[i].raw_input_value[j])); 
			psen_raw_input->sen_raw_input[i].raw_input_value[0] += psen_raw_input->sen_raw_input[i].raw_input_value[j];
			if (ret) 
			{
				goto exit;
			}
		}
	}

	for (i=0; i < psen_raw_input->sen_num; i++) {
		psen_raw_input->sen_raw_input[i].raw_input_value[0] /= 10;
	}
	
exit:         
	return ret;
#else
	mechanism_uint_sensor_data_t *punit_sensor_data = &(punit_data->unit_sensor_data);
	struct sensor_data *psen_data;
	
	psen_raw_input->sen_num = punit_sensor_data->sensor_num;
	for (i=0; i < psen_raw_input->sen_num; i++) {
		psen_raw_input->sen_raw_input[i].sen_mask = punit_sensor_data->sensor[i].sen_mask;
		psen_raw_input->sen_raw_input[i].raw_input_value[0] = 0;
	}

	for (j=1; j<=SENSOR_RAW_GET_NUM; j++) {
		for (i = 0; i < psen_raw_input->sen_num; i++) {
			psen_data = &(punit_sensor_data->sensor[i]);			
			ret = photosensor_read_input(psen_data->sen_dev.pphotosensor, &(psen_raw_input->sen_raw_input[i].raw_input_value[j]));
			psen_raw_input->sen_raw_input[i].raw_input_value[0] += psen_raw_input->sen_raw_input[i].raw_input_value[j];
			if (ret) 
			{
				goto exit;
			}
		}
	}

	for (i=0; i < psen_raw_input->sen_num; i++) {
		psen_raw_input->sen_raw_input[i].raw_input_value[0] /= 10;
	}
	
exit:         
	return ret;
#endif
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
#endif

//----------------------get feature of sensor----------------------
int mechunit_get_sensor_feature(struct mechanism_uint_data *punit_data, mech_unit_sen_feature_t *pmech_unit_sen_feature)
{
	unsigned char i;
	int32_t ret=0;
	#if 0
	pmech_unit_sen_feature->sen_num = punit_data->unit_sensor_data.sensor_num;
	for (i=0; i < pmech_unit_sen_feature->sen_num; i++) {
		pmech_unit_sen_feature->sen_feature[i].sen_mask = punit_data->unit_sensor_data.sensor[i].sen_mask;
		ret = sensor_get_feature(&(punit_data->unit_sensor_data), &pmech_unit_sen_feature->sen_feature[i]); 
		if (ret) {
			return ret;
		}
	}
	return ret;
	#else
	mechanism_uint_sensor_data_t *punit_sensor_data = &(punit_data->unit_sensor_data);
	struct sensor_data *psen_data;
	struct photosensor_feature feature;
	sen_feature_t *p_ppsfeature;
	uint32_t j;

	pmech_unit_sen_feature->sen_num = punit_sensor_data->sensor_num;
	for (i=0; i < pmech_unit_sen_feature->sen_num; i++) {
		psen_data = &(punit_sensor_data->sensor[i]);
		p_ppsfeature = &(pmech_unit_sen_feature->sen_feature[i]);
	
		p_ppsfeature->sen_mask = psen_data->sen_mask;		
		ret = photosensor_get_feature(psen_data->sen_dev.pphotosensor, &feature);
		if(!ret)
		{
			//strcpy(p_ppsfeature->sen_name, psen_data->sen_name); 
			p_ppsfeature->led_brightness_max = feature.led_brightness_max;
			p_ppsfeature->raw_input_max = feature.raw_input_max;
			p_ppsfeature->input_scale_mv = feature.input_scale_mv; 
			p_ppsfeature->calibrate_mode = feature.calibrate_mode;
		}
	}
	return ret;
	
	#endif
}

int mechunit_get_motor_feature(struct mechanism_uint_data *punit_data, mech_unit_motor_feature_t *pmech_unit_motor_feature)
{
	unsigned char i;
	int32_t ret=0;

	pmech_unit_motor_feature->motor_num = punit_data->unit_motor_data.motor_num; 
	for (i=0; i < pmech_unit_motor_feature->motor_num; i++) {
		pmech_unit_motor_feature->motor_feature[i].motor_mask = punit_data->unit_motor_data.motor[i].motor_mask; 
		ret = motor_get_feature(&(punit_data->unit_motor_data), &pmech_unit_motor_feature->motor_feature[i]); 
		if (ret) {
			return ret;
		}
	}
	return ret;
}
//--------------------------------------------
#if 0
void mechunit_get_control(struct mechanism_dev_t *pmechanism_dev)
{	
	unsigned char motor_num, sensor_num;

	motor_num = pmechanism_dev->mech_unit_data.unit_motor_data.motor_num;
	sensor_num = pmechanism_dev->mech_unit_data.unit_sensor_data.sensor_num;

	pmechanism_dev->mech_unit_control.mech_unit_motor_feature.motor_num = motor_num;
	pmechanism_dev->mech_unit_control.mech_unit_motor_feature.motor_feature = pvPortMalloc(sizeof(motor_feature_t) * motor_num); 
	
	pmechanism_dev->mech_unit_control.mech_unit_sen_feature.sen_num = sensor_num;
	pmechanism_dev->mech_unit_control.mech_unit_sen_feature.sen_feature = pvPortMalloc( sizeof(sen_feature_t) * sensor_num); 

	pmechanism_dev->mech_unit_control.mech_punit_sen_config->sen_num = sensor_num;
	pmechanism_dev->mech_unit_control.mech_punit_sen_config->sen_config = pvPortMalloc(sizeof(sen_config_t) * sensor_num); 

	pmechanism_dev->mech_unit_control.mech_unit_sen_raw_input.sen_num = sensor_num; 
	pmechanism_dev->mech_unit_control.mech_unit_sen_raw_input.sen_raw_input = pvPortMalloc(sizeof(sen_raw_input_t) * sensor_num); 
	
}
#endif


//----------------------------------------------------------
extern void stepmotor_err_callback(struct motor_data *pmotor_data, int ret);
extern int stepmotor_triger_deal(struct motor_data *pmotor_data, char triger_index, mechanism_uint_motor_data_t *punit_motor_data, mechanism_uint_sensor_data_t *punit_sensor_data);

static  void stepmotor_callback(struct motor_data *pmotor_data, struct mechanism_dev_t *pmechanism_dev, int32_t status, unsigned long stop_flag)
{
	int32_t ret;
#if 0
	if (steppermotor_is_triggersteps_done(status))  {
		//printk(KERN_DEBUG "steppermotor_is_triggersteps_done，motor_phase_accout=%d\n", pmotor_data->motor_phase_accout);
		if (pmotor_data->motor_phase_accout != 0){
			pmotor_data->phase_current_num++;
			if(pmotor_data->phase_current_num < pmotor_data->pmotor_mov->trigger_phase_num)
			{
				ret = stepmotor_triger_deal(pmotor_data, pmotor_data->phase_current_num, &(pmechanism_dev->mech_unit_data.unit_motor_data), &(pmechanism_dev->mech_unit_data.unit_sensor_data)); 
				if (ret) {
					pmotor_data->stoping_status = MOTOR_STOP_BY_ABNORMAL;
					pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
					pmotor_data->err_status = ret;
				}
			}
			
			complete(&pmotor_data->motor_phase_completion);
			
			return;
		}
		else	/*if triger int appeared and motor_phase_account==0, maybe next triger params(sensor & motor) not filled 
			or wait_for_completion_timeout not executed yet.Wait these operations completed and next triger int appeared. 2017.3.29*/
			return;
	}
     
   	if(steppermotor_is_stopped_by_total_steps(status) || steppermotor_is_stopped_by_trigger(status) ||
	   steppermotor_is_stopped_by_sensor(status)|| steppermotor_is_fault(status) ) {
		if(steppermotor_is_stopped_by_total_steps(status)){
			//printk(KERN_DEBUG "steppermotor_is_totalsteps_done\n");
			if (pmotor_data->moving_status & MOTOR_STOP_BY_SOFT_START){
				pmotor_data->stoping_status = MOTOR_STOP_BY_SOFT;
			} else
				pmotor_data->stoping_status = MOTOR_STOP_BY_TOTAL;

			pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
			pmechanism_dev->sigio_event = stop_flag|pmotor_data->stoping_status;
			
		}
		else if (steppermotor_is_stopped_by_sensor(status)) {
				//printk(KERN_DEBUG "steppermotor_is_stopped_by_sensor ");
				pmotor_data->stoping_status = MOTOR_STOP_BY_SENSOR;
			
			pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
			
			//printk(KERN_DEBUG "phase_current_num=%d stop_flag=%lx sen_mask=%x motor_sen_flag=%x\n",pmotor_data->phase_current_num,stop_flag,
			//	pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num-1].sen_mask,
			//	pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num-1].motor_sen_flag);

			pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP; 
			pmechanism_dev->sigio_event = stop_flag|pmotor_data->stoping_status | 
				MOTOR_STOP_SEN_POS_TO_RES(pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num-1].sen_pos_index) |
				MOTOR_STOP_SEN_FLAG_TO_RES(pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num-1].motor_sen_flag);
		}
		else if (steppermotor_is_stopped_by_trigger(status)) {
			//printk(KERN_DEBUG "steppermotor_is_stopped_by_trigger\n");
			pmotor_data->stoping_status = MOTOR_STOP_BY_TRIGER;
			pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
			pmechanism_dev->sigio_event = stop_flag|pmotor_data->stoping_status;
		}

		if (steppermotor_is_fault(status)) {
		    //printk(KERN_ERR "steppermotor_is_error");
		    pmotor_data->stoping_status = MOTOR_STOP_BY_ABNORMAL;
		    pmotor_data->err_status = -RESN_MECH_ERR_MOTOR_HW_ERR;
		    pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
		}

		if((pmotor_data->stoping_status & MOTOR_STOP_MASK)!=MOTOR_STOP_BY_ABNORMAL)
			#if 0	//needfill
			mechunit_sigio(pmechanism_dev);
			#else
			;
			#endif

		if (pmotor_data->motor_phase_accout != 0){
			complete(&pmotor_data->motor_phase_completion);
		}
		if (pmotor_data->motor_comp_accout != 0){
			complete_all(&(pmotor_data->motor_completion)); 
		}
	}
   
	else{
		//printk(KERN_INFO "MOTOR_INT_INVALID & status=%x motor_phase_accout=%d\n", status, pmotor_data->motor_phase_accout);
		
		pmotor_data->stoping_status = MOTOR_STOP_BY_ABNORMAL;
		pmotor_data->err_status = -RESN_MECH_ERR_MOTOR_INT_INVALID;
		pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;

		if (pmotor_data->motor_phase_accout != 0)
			complete(&pmotor_data->motor_phase_completion);
		if (pmotor_data->motor_comp_accout != 0)
			complete_all(&(pmotor_data->motor_completion));
	}
	#else
	pmotor_data->stoping_status = MOTOR_STOP_BY_TOTAL;
	pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
	pmechanism_dev->sigio_event = stop_flag|pmotor_data->stoping_status;
	if (pmotor_data->motor_phase_accout != 0)
		complete(&pmotor_data->motor_phase_completion);
	if (pmotor_data->motor_comp_accout != 0)
		complete_all(&(pmotor_data->motor_completion));
	pmechanism_dev->sigio_event += RESN_OFFSET_MECH_DRIVER_MOV;
	#endif
}
void mech_stepmotor_callback(struct steppermotor *motor, struct callback_data *pcall_back_data)
{
	struct motor_data *pmotor_data;
	int status;
	static unsigned char i=0;
	int motor_mask = pcall_back_data->data1; 
	struct mechanism_dev_t *pmech_dev = (struct mechanism_dev_t *)(pcall_back_data->data2); 

	motor_get_data(&pmech_dev->mech_unit_data.unit_motor_data, motor_mask, pmotor_data, i); 
	if (i == pmech_dev->mech_unit_data.unit_motor_data.motor_num) {
		pmech_dev->sigio_event = MOTOR_STOP_BY_ABNORMAL|MOTOR_STOP_UINT_TO_RES(motor_mask);
		#if 0	//needfill
		mechunit_sigio(pmech_dev);
		#endif
		return;
	}

	//to avoid invalid interrupts after motor stoped
	if(!(pmotor_data->moving_status & MOTOR_MOVE_STATUS_RUNNING))
		return;

	status = steppermotor_status(pmotor_data->motor_dev.psteppermotor);
    
	stepmotor_callback(pmotor_data, pmech_dev, status, MOTOR_STOP_UINT_TO_RES(motor_mask));
    
}

#if 0
void mech_dcmotor_callback(struct dcmotor *motor,struct callback_data *pcall_back_data)
{
	struct motor_data *pmotor_data;
	int32_t status;
	static unsigned char i=0;
	int32_t motor_mask = pcall_back_data->data1; 
	struct mechanism_dev_t *pmech_dev = (struct mechanism_dev_t *)(pcall_back_data->data2); 

	//printk(KERN_DEBUG "mechunit_dcmotor_callback...........pmech_dev=%x motor_mask=%x\n", (int)pmech_dev, motor_mask);
 
	motor_get_data(&pmech_dev->mech_unit_data.unit_motor_data, motor_mask, pmotor_data, i); 
	if (i == pmech_dev->mech_unit_data.unit_motor_data.motor_num) {
		pmech_dev->sigio_event = MOTOR_STOP_BY_ABNORMAL | MOTOR_STOP_UINT_TO_RES(motor_mask);
		#if 0	//needfill
		mechunit_sigio(pmech_dev);
		#endif
		return;
	}
	  
	//printk(KERN_DEBUG "mechunit_dcmotor_callback:phase_current_num=%d moving_status=%x\n",pmotor_data->phase_current_num, pmotor_data->moving_status);
	if(!(pmotor_data->moving_status & MOTOR_MOVE_STATUS_RUNNING))
	{
		return;
	}

	status = dcmotor_status(pmotor_data->motor_dev.pdcmotor);
    
	//printk(KERN_DEBUG "mechunit_dcmotor_callback:status=%x, pmotor_data=%x, motor_comp_accout=%x \n", status, (int)pmotor_data, pmotor_data->motor_comp_accout);
  
	if ((!dcmotor_is_running(status))||(dcmotor_is_stopped_by_sensor(status))||dcmotor_is_error(status)){
		if (!dcmotor_is_running(status))
		{
			if (pmotor_data->moving_status & MOTOR_STOP_BY_SOFT_START){
				pmotor_data->stoping_status = MOTOR_STOP_BY_SOFT;
			} else
				pmotor_data->stoping_status = MOTOR_STOP_BY_TOTAL;
			pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
			pmech_dev->sigio_event = pmotor_data->stoping_status|MOTOR_STOP_UINT_TO_RES(motor_mask);
			//printk(KERN_DEBUG "mechunit_dcmotor_callback1:sigio_event=%lx ", pmech_dev->sigio_event);
		}
		if ((dcmotor_is_stopped_by_sensor(status))) {
			pmotor_data->stoping_status = MOTOR_STOP_BY_SENSOR;
			pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
			pmech_dev->sigio_event = pmotor_data->stoping_status | MOTOR_STOP_UINT_TO_RES(motor_mask) |
				MOTOR_STOP_SEN_POS_TO_RES(pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num-1].sen_pos_index) |
				MOTOR_STOP_SEN_FLAG_TO_RES(pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num-1].motor_sen_flag);
			//printk(KERN_DEBUG "mechunit_dcmotor_callback2:sigio_event=%lx sen_mask=%x sen_pos_index=%d\n", pmech_dev->sigio_event, 
				//pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num - 1].sen_mask, 
				//pmotor_data->pmotor_mov->motor_trigger_phase[pmotor_data->phase_current_num - 1].sen_pos_index); 
		}
		if (dcmotor_is_error(status))
		{
			pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
			pmotor_data->stoping_status = MOTOR_STOP_BY_ABNORMAL; 
			pmotor_data->err_status = -RESN_MECH_ERR_MOTOR_HW_ERR;
		}
		if((pmotor_data->stoping_status & MOTOR_STOP_MASK)!=MOTOR_STOP_BY_ABNORMAL)
			#if 0	//needfill
			mechunit_sigio(pmech_dev);
			#else
			;
			#endif

		if (pmotor_data->motor_phase_accout != 0)
			complete(&pmotor_data->motor_phase_completion);

		if (pmotor_data->motor_comp_accout != 0)
			complete_all(&(pmotor_data->motor_completion)); 
		
	}
	else{
		pmotor_data->stoping_status = MOTOR_STOP_BY_ABNORMAL; 
		pmotor_data->moving_status = MOTOR_MOVE_STATUS_STOP;
		pmotor_data->err_status = -RESN_MECH_ERR_MOTOR_INT_INVALID;

		if (pmotor_data->motor_phase_accout != 0)
			complete(&pmotor_data->motor_phase_completion);
		if (pmotor_data->motor_comp_accout != 0)
			complete_all(&(pmotor_data->motor_completion));
	}

	//printk(KERN_DEBUG "mechunit_dcmotor_callback...........over! mech_sigio=%lx\n", pmech_dev->sigio_event);
    
}
#endif

//----------------------------------------------------------
static int32_t mech_motor_init(mechanism_uint_motor_data_t *punit_motor_data, unsigned short motor_mask, motor_callback_t *pcallback, struct mechanism_dev_t *mech_dev)
{
	struct motor_data *pmotor_data;
	unsigned char i;
	struct callback_data call_back_data;
	
	//printk(KERN_DEBUG "mechunit_motor_init.............mech_dev=%x motor_mask=%x\n", (int)mech_dev, motor_mask);

	motor_get_data(punit_motor_data, motor_mask, pmotor_data, i);
	if (i==punit_motor_data->motor_num) {
		return -RESN_MECH_ERR_MOTOR_GETDATA;
	}
	
	call_back_data.data1 = motor_mask;
	call_back_data.data2 = (int)mech_dev;

	switch(pmotor_data->motor_type)
	{
	case STEPPERMOTOR:	
		pmotor_data->callback.steppermotor_callback = pcallback->steppermotor_callback;
		steppermotor_set_callback(pmotor_data->motor_dev.psteppermotor, pcallback->steppermotor_callback, &call_back_data); 
		break;
	#if 0
	case BRUSH_DCMOTOR:
		pmotor_data->callback.dcmotor_callback = pcallback->dcmotor_callback;
		dcmotor_set_callback(pmotor_data->motor_dev.pdcmotor, pcallback->dcmotor_callback, &call_back_data); 
		break;
	#endif
	default:
		return -1;
	}

	init_completion(&(pmotor_data->motor_completion));
	pmotor_data->motor_comp_accout = 0;

	init_completion(&(pmotor_data->motor_phase_completion));
	pmotor_data->motor_phase_accout = 0; 

	pmotor_data->moving_status = MOTOR_MOVE_STATUS_INIT;
	pmotor_data->stoping_status = 0;
	return 0;
}


int32_t 	mechunit_init(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control)
{
	int32_t ret=0, i;
	motor_callback_t callback;

	for (i=0; i < mech_dev->mech_unit_data.unit_motor_data.motor_num; i++) {
		if (mech_dev->mech_unit_data.unit_motor_data.motor[i].motor_type==STEPPERMOTOR) {
			callback.steppermotor_callback = mech_stepmotor_callback;
		}
		//else
		//	callback.dcmotor_callback = mech_dcmotor_callback;

		ret = mech_motor_init(&mech_dev->mech_unit_data.unit_motor_data,
			mech_dev->mech_unit_data.unit_motor_data.motor[i].motor_mask, 
			&callback, mech_dev); 
	
		if (ret)
		{
			return ret;
		}
	}

	return ret;
}

int32_t	mechunit_motor_move(struct mechanism_dev_t * mech_dev, mech_control_t *mech_control)
{
	int32_t ret;
	mechunit_motor_mov_t  *pmech_motor_move;

	pmech_motor_move = (mechunit_motor_mov_t *)mech_control->buffer;

	ret = motor_move_init(&mech_dev->mech_unit_data.unit_motor_data, pmech_motor_move->motor_mask);
	if (ret) 
		goto end_mech_motor_move;
	
	mech_dev->sigio_event = 0;
	ret = motor_start(&mech_dev->mech_unit_data.unit_motor_data, &mech_dev->mech_unit_data.unit_sensor_data,
		pmech_motor_move->motor_mask, pmech_motor_move->dir, pmech_motor_move->pmotor_mov);
	if (ret) 
	{
		goto end_mech_motor_move;
	}
	
	ret = motor_wait_stop(&mech_dev->mech_unit_data.unit_motor_data, 
		&mech_dev->mech_unit_data.unit_sensor_data,
		pmech_motor_move->motor_mask); 
	
	motor_end(&mech_dev->mech_unit_data.unit_motor_data, &mech_dev->mech_unit_data.unit_sensor_data,pmech_motor_move->motor_mask);
end_mech_motor_move:
	return ret;
}

int32_t	mechunit_stop(struct mechanism_dev_t *mech_dev, mech_control_t *pmech_control)
{
	unsigned short motor_mask = (unsigned short)pmech_control->buffer;

	motor_stop(&mech_dev->mech_unit_data.unit_motor_data, motor_mask); 
		
	return 0;
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

