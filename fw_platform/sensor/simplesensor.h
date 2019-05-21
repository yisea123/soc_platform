/*
 * Simple Sensor device driver definitions
 *
 * Copyright 2018,2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __SIMPLESENSOR_H__
#define __SIMPLESENSOR_H__

#include <stddef.h>
#include <stdint.h>
#include <sensor.h>


/* definition of simplesensor status */
#define SIMPLESENSOR_DETECTED			(1 << 0)

/* macros to test simplesensor status */
#define simplesensor_is_detected(s)		((s) & SIMPLESENSOR_DETECTED)


/*
 * struct simplesensor
 *
 * One for each simplesensor device.
 */
struct simplesensor {
	const void *resource;			// pointer to the resource block of a simplesensor instance
	int (*const install)(struct simplesensor *sensor);	// pointer to the device installation function 
	sensor_status_mapping_t status_mapping;
	const struct simplesensor_ops *ops;
	/* simplesensor trigger definition */
	sensor_event_t eventtrigger;
	void (*eventhandler)(struct simplesensor *sensor, sensor_event_t event, void *data);	// callback handling simplesensor events
	void *handlerdata;
};


/*
 * struct simplesensor_ops - simplesensor operations
 * @status: get status of this simplesensor
 * @enable: enable this simplesensor
 * @disable: diable this simplesensor
 * @set_event: set event handle of this simplesensor
 * @unset_event: unset event handle of this simplesensor
 */
struct simplesensor_ops {
	int	(*status)(struct simplesensor *sensor, int *status);

	int	(*enable)(struct simplesensor *sensor);
	int	(*disable)(struct simplesensor *sensor);

	int	(*read_input)(struct simplesensor *sensor, uint32_t *val);
	int	(*set_event)(struct simplesensor *sensor, sensor_event_t event, void (*eventhandle)(struct simplesensor *, sensor_event_t, void *), void *data);
	int	(*unset_event)(struct simplesensor *sensor, sensor_event_t event);
};


/* declaration of simplesensor list */
extern struct simplesensor *simplesensor_list[];
extern const int simplesensor_num;


/* function prototypes of simplesensor driver */
extern struct simplesensor *simplesensor_get(int index);
extern int simplesensor_install_devices(void);

extern int simplesensor_enable(struct simplesensor *sensor);
extern int simplesensor_disable(struct simplesensor *sensor);
extern int simplesensor_status(struct simplesensor *sensor, int *status);
extern int simplesensor_read_input(struct simplesensor *sensor, uint32_t *val);

extern int simplesensor_set_event(struct simplesensor *sensor, sensor_event_t event, void (*eventhandle)(struct simplesensor *, sensor_event_t, void *), void *data);
extern int simplesensor_unset_event(struct simplesensor *sensor, sensor_event_t event);


#endif /* __SIMPLESENSOR_H__ */
