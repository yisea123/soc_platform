/*
 * Photo Sensor device driver definitions
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __PHOTOSENSOR_H__
#define __PHOTOSENSOR_H__

#include <stddef.h>
#include <stdint.h>

typedef enum {
	PHOTOSENSOR_DIGITAL,
	PHOTOSENSOR_ANALOG
} sensor_type;


typedef enum{
	PHOTOSENSOR_THROUGHBEAM,
	PHOTOSENSOR_REFLECTIVE
}sensor_mode_t;

typedef enum{
	PHOTOSENSOR_0MEANS_DETECTED,
	PHOTOSENSOR_1MEANS_DETECTED
}sensor_polarity_t;

/* definition of photosensor status */
#define PHOTOSENSOR_COVERED			(1 << 0)

/* macros to test photosensor status */
#define photosensor_is_covered(s)		((s) & PHOTOSENSOR_COVERED)


#define MINIMUM_BRIGHTNESS	0
#define MAXIMUM_BRIGHTNESS	255


/* photosensor trigger configuration block */
struct sensor_trigger {
	int mode;				// trigger mode: 0 - from COVERED to UNCOVERED; 1 from UNCOVERED to COVERED;
	int enable;				// trigger enable flag: 0 - disabled; 1 - enabled
};


/* photosensor configuration block */
struct photosensor_config {
	int led_brightness;			// LED brightness level of photosensor: should be at [0,255]
	struct sensor_trigger trigger;		// photosensor trigger information
	unsigned long compare_threshold;	// ADC compare threshold value (for analog sensor ONLY)
};


/* photosensor feature block */
struct photosensor_feature {
	int led_brightness_max;			// maximum LED brightness level of photosensor
	unsigned long raw_input_max;		// maximum raw input value
	int input_scale_mv;			// scale to convert raw input value to voltage (mV)
	int covered_mode;
	int calibrate_mode;			//0——only without paper， 1——only with paper
};


/*
 * struct photosensor
 *
 * One for each photosensor device.
 */
struct photosensor {
	const void *resource;			// pointer to the resource block of a photosensor instance
	int (*const install)(struct photosensor *sensor);	// pointer to the device installation function 
	struct photosensor_feature feature;
	struct photosensor_config config;	// copy of current photosensor configuration
	sensor_type type;
	sensor_mode_t sensor_mode;
	sensor_polarity_t sensor_polarity;
	const struct photosensor_ops *ops;
	void (*callback)(struct photosensor *sensor, void *data);	// callback handling photosensor events
	void *callbackdata;
};


/*
 * struct photosensor_ops - photosensor operations
 * @status: get status of this photosensor
 * @enable: enable this photosensor
 * @disable: diable this photosensor
 * @get_config: get configure of this photosensor
 * @set_config: set configure of this photosensor
 * @read_input: read raw input value of this photosensor
 */ 
struct photosensor_ops {
	int	(*config)(struct photosensor *sensor, const struct photosensor_config *config);
	int	(*status)(struct photosensor *sensor, int *status);

	int	(*enable)(struct photosensor *sensor);
	int	(*disable)(struct photosensor *sensor);

	int	(*read_input)(struct photosensor *motor, uint32_t *val);
};


static inline sensor_type photosensor_type(struct photosensor *sensor)
{
	return (!sensor) ? (sensor_type)-1 : sensor->type;
}


/* declaration of photosensor list */
extern struct photosensor photosensor_list[];
extern const int photosensor_num;


/* function prototypes of photosensor driver */
extern struct photosensor *photosensor_get(int index);
extern int photosensor_install_devices(void);

extern int photosensor_enable(struct photosensor *sensor);
extern int photosensor_disable(struct photosensor *sensor);
extern int photosensor_status(struct photosensor *sensor, int *status);
extern int photosensor_read_input(struct photosensor *sensor, uint32_t *val);
extern int photosensor_get_feature(struct photosensor *sensor, struct photosensor_feature *feature);

extern int photosensor_set_callback(struct photosensor *sensor, void (*callback)(struct photosensor *, void *), void *data);

extern int photosensor_get_config(struct photosensor *sensor, struct photosensor_config *config);
extern int photosensor_set_config(struct photosensor *sensor, const struct photosensor_config *config);


#endif /* __PHOTOSENSOR_H__ */
