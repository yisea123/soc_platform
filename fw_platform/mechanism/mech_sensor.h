#ifndef __MECH_SENSOR_H__
#define __MECH_SENSOR_H__

#include <stdlib.h>
#include "m2sxxx.h"
#include "mechlib.h"
#include "photosensor.h"

//---------------------------------------
#define SEN_MEDIA_IN	0
#define SEN_MEDIA_OUT	1

//-------------------------------------------------------
// database struct of sensors in a mechanism unit.
struct sensor_data{
    uint32_t sen_mask;
    //uint8_t sen_name[MECHUINT_NAME_LEN];
    union{
        struct photosensor *pphotosensor;
    }sen_dev;
    struct photosensor_config config;
};

typedef struct {
	uint8_t  sensor_num;
	struct sensor_data *sensor;
	uint32_t sensor_masks;
}mechanism_uint_sensor_data_t;

#define sensor_get_data(punit_sensor_data, senmask, psen_data, i) \
     for(i=0; i<(punit_sensor_data)->sensor_num; i++) \
        if((punit_sensor_data)->sensor[i].sen_mask== (senmask)) \
        {\
            psen_data = &((punit_sensor_data)->sensor[i]); \
            break;\
        } \


//----------------------------------------------------
#if 0
//----------------------sensor init----------------------
extern int32_t sensor_init(mechanism_uint_sensor_data_t *punit_sensor_data,  uint32_t sen_masks);
#endif
//----------------------sensor enable/disable---------------------- 
extern int32_t sensor_enable(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_masks, uint8_t enable);

//----------------------get the output value of sensor----------------------
extern int32_t sensor_get_val(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, uint32_t *val);
//----------------------get the status of sensor(detected or undetected)----------------------
//extern int32_t sensor_get_appval(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, uint32_t *appval);
#if 0
//----------------------set sensor trigger----------------------
extern int sensor_set_trigger(mechanism_uint_sensor_data_t *punit_sensor_data, unsigned int sen_masks, unsigned char trigger_type);
//----------------------set sensor next trigger----------------------
extern int sensor_set_trigger_next(mechanism_uint_sensor_data_t *punit_sensor_data, unsigned int sen_masks, unsigned char trigger_type);
//----------------------clear sensor next trigger----------------------
extern int sensor_clear_trigger_next(mechanism_uint_sensor_data_t *punit_sensor_data, unsigned int sen_masks);
#endif
//----------------------set sensor config----------------------
extern int sensor_set_config(mechanism_uint_sensor_data_t *punit_sensor_data, mech_unit_sen_config_t *pmech_unit_sen_config, sen_config_t *p_sen_config );
//----------------------get sensor feature----------------------
//extern int sensor_get_feature(mechanism_uint_sensor_data_t *punit_sensor_data, sen_feature_t *p_ppsfeature);

extern int32_t sensor_set_evnet(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, sensor_event_t event, void (*eventhandle)(struct photosensor *, sensor_event_t, void *), void *data);
//===================================================
#if 0
// 启动到达传感器检测
extern uint8_t sensor_arrive_set(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask,sensor_irq_handler_t handler, void * dev_id);

// 启动离开传感器检测
extern 	uint8_t sensor_leave_set(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask,sensor_irq_handler_t handler, void * dev_id);

// 关闭传感器检测
extern uint8_t sensor_isr_disable(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask);
#endif

#endif

