#include "mech_sensor.h"
#include "respcode.h"

#define for_each_mask_of_unit(masks, mask, i) \
	for (i=0, mask = masks&0x01; i<32; i++, mask = (masks & (0x01<<i)))
	//for (i=0, mask = masks&0x01; (i<16)&&masks; i++, mask = (masks & (0x01<<i)), masks &= ~(0x01<<i))

#if 0
//----------------------sensor init----------------------
int32_t sensor_init(mechanism_uint_sensor_data_t *punit_sensor_data,  uint32_t sen_masks)
{
	struct sensor_data *psen_data;
	uint32_t sen_mask, i,j;
	int32_t ret=0;

	for_each_mask_of_unit(sen_masks, sen_mask, i) {
		if (!sen_mask) continue;
		
		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
		    return -RESN_MECH_ERR_SENSOR_GETDATA;
		}
		
		ret = photosensor_get_config(psen_data->sen_dev.pphotosensor, &(psen_data->config));
		if (ret) {
			return ret;
		}

		#if 0
		psen_data->config.trigger.mode = 0;
		psen_data->config.trigger.enable = 0;
		#endif
		ret = photosensor_set_config(psen_data->sen_dev.pphotosensor, &(psen_data->config));
		if (ret) {
		    return ret;
		}
	}
	return 0;
}
#endif
//----------------------sensor enable/disable---------------------- 
int32_t sensor_enable(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_masks, uint8_t enable)
{
	struct sensor_data *psen_data;
	uint32_t sen_mask, i, j;
	int32_t ret=0;

	for_each_mask_of_unit(sen_masks, sen_mask, i) {
		if (!sen_mask) continue;

		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
			return -RESN_MECH_ERR_SENSOR_GETDATA;
		}

		//printk(KERN_DEBUG "sensor_enable: mask=%x enable=%x\n", sen_mask, enable);

		if (!enable){
			photosensor_disable(psen_data->sen_dev.pphotosensor); 
		}
		else
			ret = photosensor_enable(psen_data->sen_dev.pphotosensor); 

		if (ret) {
			return ret;
		}
	}
	return 0;
}


//----------------------get the output value of sensor----------------------
int32_t sensor_get_val(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, uint32_t *val)
{
	struct sensor_data *psen_data;
	unsigned int j;

	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
        if (j==punit_sensor_data->sensor_num) {
            return -RESN_MECH_ERR_SENSOR_GETDATA;
        }
       
	return photosensor_read_input(psen_data->sen_dev.pphotosensor, val);
}

int32_t sensor_set_evnet(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, sensor_event_t event, void (*eventhandle)(struct photosensor *, sensor_event_t, void *), void *data)
{
	struct sensor_data *psen_data;
	unsigned int j;

	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
        if (j==punit_sensor_data->sensor_num) {
            return -RESN_MECH_ERR_SENSOR_GETDATA;
        }
	return photosensor_set_event(psen_data->sen_dev.pphotosensor, event, eventhandle, data);
}

int32_t sensor_unset_evnet(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, sensor_event_t event)
{
	struct sensor_data *psen_data;
	unsigned int j;

	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
        if (j==punit_sensor_data->sensor_num) {
            return -RESN_MECH_ERR_SENSOR_GETDATA;
        }
	return photosensor_unset_event(psen_data->sen_dev.pphotosensor, event);
}

#if 0
//----------------------get the status of sensor(detected or undetected)----------------------
int32_t sensor_get_appval(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, uint32_t *appval)
{
	struct sensor_data *psen_data;
	unsigned int j;
	int32_t ret=0;

	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
	if (j==punit_sensor_data->sensor_num) {
		return -RESN_MECH_ERR_SENSOR_GETDATA;
	}
	ret =  photosensor_status(psen_data->sen_dev.pphotosensor, appval); 
	return ret;
}
#endif

#if 0
//----------------------set sensor trigger----------------------
int sensor_set_trigger(mechanism_uint_sensor_data_t *punit_sensor_data, unsigned int sen_masks, unsigned char trigger_type)
{
	struct sensor_data *psen_data;
	unsigned int sen_mask, i, j;
	int ret=0;

	for_each_mask_of_unit(sen_masks, sen_mask, i) {
		if (!sen_mask) continue;

		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
		    return -RESN_MECH_ERR_SENSOR_GETDATA;
		}
        
	
		ret = photosensor_get_config(psen_data->sen_dev.pphotosensor, &(psen_data->config));
		if (ret) {
			return ret;
		}

		if (trigger_type == SEN_MEDIA_IN) {
			psen_data->config.trigger.mode = 1;
		}
		else
			psen_data->config.trigger.mode = 0;

		psen_data->config.trigger.enable = 1;
		ret = photosensor_set_config(psen_data->sen_dev.pphotosensor, &(psen_data->config));
	}
	return ret;
}

//----------------------set sensor next trigger----------------------
int sensor_set_trigger_next(mechanism_uint_sensor_data_t *punit_sensor_data, unsigned int sen_masks, unsigned char trigger_type)
{
	struct sensor_data *psen_data;
	unsigned int sen_mask, i, j;
	int ret=0;
	struct sensor_trigger trigger;

	for_each_mask_of_unit(sen_masks, sen_mask, i) {
		if (!sen_mask) continue;

		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
			return -RESN_MECH_ERR_SENSOR_GETDATA;
		}
       
		trigger.mode = trigger_type;
		trigger.enable = 1;
		ret = photosensor_set_trigger_next(psen_data->sen_dev.pphotosensor, &trigger);
		if (!ret)
			continue;
	}
	return ret;
}

int sensor_clear_trigger_next(mechanism_uint_sensor_data_t *punit_sensor_data, unsigned int sen_masks)
{
	struct sensor_data *psen_data;
	unsigned int sen_mask, i, j;
	int ret=0;

	for_each_mask_of_unit(sen_masks, sen_mask, i) {
		if (!sen_mask) continue;

		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
			return -RESN_MECH_ERR_SENSOR_GETDATA;
		}

		ret = photosensor_clear_trigger_next(psen_data->sen_dev.pphotosensor);
		if (ret) {
			return ret;
		    }

	}
	return 0;
}
#endif
//----------------------set sensor config----------------------
int32_t sensor_set_config(mechanism_uint_sensor_data_t *punit_sensor_data, mech_unit_sen_config_t *pmech_unit_sen_config, sen_config_t *p_sen_config )
{
	struct sensor_data *psen_data;
	struct photosensor_config config;
	unsigned int sen_mask, i, j, k;
	int32_t ret=0;

	for_each_mask_of_unit(p_sen_config->sen_mask, sen_mask, i) {
		if (!sen_mask) continue;

		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
			return -RESN_MECH_ERR_SENSOR_GETDATA;
		}

		ret = photosensor_get_config(psen_data->sen_dev.pphotosensor, &config);
		if (ret) {
			return ret;
		}
             
		config.compare_threshold = p_sen_config->pps_ref_value; 
		config.led_brightness = p_sen_config->pps_drv_value; 
		#if 0
		config.trigger.enable = p_sen_config->trigger_enable; 
		config.trigger.mode = p_sen_config->trigger_mode; 
		#endif
		for (k=0; k < pmech_unit_sen_config->sen_num; k++) {
			if (pmech_unit_sen_config->sen_config[k].sen_mask==sen_mask) {
			    pmech_unit_sen_config->sen_config[k].pps_drv_value = config.led_brightness;
			    pmech_unit_sen_config->sen_config[k].pps_ref_value = config.compare_threshold;
				#if 0
			    pmech_unit_sen_config->sen_config[k].trigger_enable = config.trigger.enable;
			    pmech_unit_sen_config->sen_config[k].trigger_mode = config.trigger.mode;
				#endif
			    break;
			}
		}
		if (k == pmech_unit_sen_config->sen_num) {
			return -1;
		}
		ret = photosensor_set_config(psen_data->sen_dev.pphotosensor, &config);

		if (ret) 
			return ret;
	    
	}
	return ret;
}

#if 0
//----------------------get feature of sensor----------------------
int32_t sensor_get_feature(mechanism_uint_sensor_data_t *punit_sensor_data, sen_feature_t *p_ppsfeature)
{
	struct sensor_data *psen_data;
	int32_t ret;
	struct photosensor_feature feature;
	uint32_t j;

	sensor_get_data(punit_sensor_data, p_ppsfeature->sen_mask, psen_data, j);
	if (j==punit_sensor_data->sensor_num) {
		return -RESN_MECH_ERR_SENSOR_GETDATA;
	}

	ret = photosensor_get_feature(psen_data->sen_dev.pphotosensor, &feature);
	if(!ret)
	{
		strcpy(p_ppsfeature->sen_name, psen_data->sen_name); 
		p_ppsfeature->led_brightness_max = feature.led_brightness_max;
		p_ppsfeature->raw_input_max = feature.raw_input_max;
		p_ppsfeature->input_scale_mv = feature.input_scale_mv; 
		p_ppsfeature->calibrate_mode = feature.calibrate_mode;
	}
	return ret;
}
#endif
//===================================================
#if 0
// 启动到达传感器检测
uint8_t sensor_arrive_set(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask,sensor_irq_handler_t handler, void * dev_id)
{
	struct sensor_data *psen_data;
	unsigned int j;

	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
        if (j==punit_sensor_data->sensor_num) {
            return -RESN_MECH_ERR_SENSOR_GETDATA;
        }

	photosensor_set_callback(psen_data->sen_dev.pphotosensor, handler, dev_id, TRIGER_RISING);
	return 0;
}

// 启动离开传感器检测
uint8_t sensor_leave_set(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask,sensor_irq_handler_t handler, void * dev_id)
{
	struct sensor_data *psen_data;
	unsigned int j;
	
	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
	if (j==punit_sensor_data->sensor_num) {
		return -RESN_MECH_ERR_SENSOR_GETDATA;
	}
	#if 0// NEEDFILL_20180710
	sensorint_handler_register(sen_mask, handler, dev_id, TRIGER_FALLING, punit_sensor_data->sensor->sen_dev.pphotosensor->sensordev->pfpga_interrupt_source);
	#endif
	return 0;
}


// 关闭传感器检测
uint8_t sensor_isr_disable(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask)
{
	struct sensor_data *psen_data;
	unsigned int j;
	
	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
	if (j==punit_sensor_data->sensor_num) {
		return -RESN_MECH_ERR_SENSOR_GETDATA;
	}
	#if 0// NEEDFILL_20180710
	sensorint_handler_unregister(sen_mask, punit_sensor_data->sensor->sen_dev.pphotosensor->sensordev->pfpga_interrupt_source);
	#endif
	return 0;
}
#endif

