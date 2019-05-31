#include "mech_sensor2.h"
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

#if 0
	for_each_mask_of_unit(sen_masks, sen_mask, i) {
#else
	uint32_t tmp_masks, tmp;

	i = 0;
	tmp_masks = sen_masks;
	while(tmp_masks)
	{
		tmp = 0x01<<i;
		sen_mask = tmp_masks&tmp;
		tmp_masks &= ~tmp;
		i++;
#endif
	
		if (!sen_mask) continue;

		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
			return -RESN_MECH_ERR_SENSOR_GETDATA;
		}

		//printk(KERN_DEBUG "sensor_enable: mask=%x enable=%x\n", sen_mask, enable);

		if(psen_data->sensor_type == SENSOR_PHOTO_TYPE)
		{
			if (!enable){
				photosensor_disable(psen_data->sen_dev.pphotosensor); 
			}
			else
				ret = photosensor_enable(psen_data->sen_dev.pphotosensor); 
			
			if (ret) {
				return ret;
			}
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

	switch(psen_data->sensor_type)
	{
	case SENSOR_PHOTO_TYPE:
		return photosensor_read_input(psen_data->sen_dev.pphotosensor, val);
	case SENSOR_MICROSWITCH:
		return simplesensor_read_input(psen_data->sen_dev.psimplesensor, val);
	default:
		return -1;
	}
	
}

void photosensor_eventhandle(struct photosensor *sensor, sensor_event_t event, void *data)
{
	struct sensor_data *psen_data = (struct sensor_data *)data;

	if(psen_data->eventhandle)
		psen_data->eventhandle();
}

void simplesensor_eventhandle(struct simplesensor *sensor, sensor_event_t event, void *data)
{
	struct sensor_data *psen_data = (struct sensor_data *)data;

	if(psen_data->eventhandle)
		psen_data->eventhandle();
}

int32_t sensor_set_event(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, sensor_event_t event, void (*eventhandle)(void))
{
	struct sensor_data *psen_data;
	unsigned int j;

	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
        if (j==punit_sensor_data->sensor_num) {
            return -RESN_MECH_ERR_SENSOR_GETDATA;
        }
	psen_data->eventhandle = eventhandle;

	switch(psen_data->sensor_type)
	{
	case SENSOR_PHOTO_TYPE:
		return photosensor_set_event(psen_data->sen_dev.pphotosensor, event, photosensor_eventhandle, (void *)psen_data);
	case SENSOR_MICROSWITCH:
		return simplesensor_set_event(psen_data->sen_dev.psimplesensor, event, simplesensor_eventhandle, (void *)psen_data);
	default:
		return -1;
	}
}

int32_t sensor_unset_event(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, sensor_event_t event)
{
	struct sensor_data *psen_data;
	unsigned int j;

	//printf("sensor_unset_event1\n\r");
	sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
        if (j==punit_sensor_data->sensor_num) {
            return -RESN_MECH_ERR_SENSOR_GETDATA;
        }
	psen_data->eventhandle = NULL;

	switch(psen_data->sensor_type)
	{
	case SENSOR_PHOTO_TYPE:
		//printf("sensor_unset_event2\n\r");
		return photosensor_unset_event(psen_data->sen_dev.pphotosensor, event);
	case SENSOR_MICROSWITCH:
		//printf("sensor_unset_event3\n\r");
		return simplesensor_unset_event(psen_data->sen_dev.psimplesensor, event);
	default:
		return -1;
	}	
		
}

//----------------------set sensor config----------------------
int32_t sensor_set_config(mechanism_uint_sensor_data_t *punit_sensor_data, mech_unit_sen_config_t *pmech_unit_sen_config, sen_config_t *p_sen_config )
{
	struct sensor_data *psen_data;
	struct photosensor_config config;
	unsigned int sen_mask, i, j, k;
	int32_t ret=0;

#if 0
	for_each_mask_of_unit(p_sen_config->sen_mask, sen_mask, i) {
	if (!sen_mask) continue;
#else
	sen_mask = p_sen_config->sen_mask;
	{
#endif
		//printf("sensor_set_config1\n\r");
		sensor_get_data(punit_sensor_data, sen_mask, psen_data, j);
		if (j==punit_sensor_data->sensor_num) {
			return -RESN_MECH_ERR_SENSOR_GETDATA;
		}

		if(psen_data->sensor_type != SENSOR_PHOTO_TYPE)
		#if 0	
			continue;
		#else
			return ret;
		#endif

		//printf("sensor_set_config2\n\r");
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
		//printf("sensor_set_config3\n\r");
		ret = photosensor_set_config(psen_data->sen_dev.pphotosensor, &config);

		if (ret) 
			return ret;
	    
	}
	//printf("sensor_set_config4\n\r");
	return ret;
}



