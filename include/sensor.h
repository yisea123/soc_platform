#ifndef __SENSOR_H__
#define __SENSOR_H__


/* logical sensor status definition */
typedef enum {
	SENSOR_ST_UNDETECTED,
	SENSOR_ST_DETECTED
} sensor_status_t;


/* low-lever hardware sensor (GPIO, ADC input) status mapping to logical status */
typedef enum {
	SENSOR_ST_DETETED_IS_HIGHLEVEL,
	SENSOR_ST_DETETED_IS_LOWLEVEL,
	SENSOR_ST_DETETED_IS_ABOVE_THRESHOLD,
	SENSOR_ST_DETETED_IS_BELOW_THRESHOLD,
} sensor_status_mapping_t;


/* logical sensor event definition */
typedef enum {
	SENSOR_EV_NONE,
	SENSOR_EV_DETECTED,
	SENSOR_EV_UNDETECTED,
	SENSOR_EV_EITHER,
} sensor_event_t;


#endif /* __SENSOR_H__ */
