#ifndef __MECH_SENSOR_H__
#define __MECH_SENSOR_H__

#include <stdlib.h>
#include "m2sxxx.h"
#include "mechlib2.h"
#include "photosensor.h"

//---------------------------------------
#define SEN_MEDIA_IN	0
#define SEN_MEDIA_OUT	1

typedef enum{
	SENSOR_PHOTO_TYPE,
	SENSOR_MICROSWITCH
}sensor_type_t;
//-------------------------------------------------------
// database struct of sensors in a mechanism unit.
struct sensor_data{
	char sen_name[MECHUINT_NAME_LEN];
	uint32_t sen_mask;
	sensor_type_t sensor_type;
	union{
	struct photosensor *pphotosensor;
	struct simplesensor *psimplesensor;
	}sen_dev;
	struct photosensor_config config;
	void (*eventhandle)(void);
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

extern int32_t sensor_set_event(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, sensor_event_t event, void (*eventhandle)(void));
extern int32_t sensor_unset_event(mechanism_uint_sensor_data_t *punit_sensor_data, uint32_t sen_mask, sensor_event_t event);

#endif

