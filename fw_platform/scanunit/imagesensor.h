/*
 * Image Sensor device driver definitions
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __IMAGESENSOR_H__
#define __IMAGESENSOR_H__

#include <stddef.h>
#include <stdint.h>
#include "scanunit.h"


/*
 * struct imagesensor
 *
 * One for each imagesensor device.
 */
struct imagesensor {
	const void *resource;				// pointer to the resource block of an imagesensor instance
	int (*const install)(struct imagesensor *sensor);	// pointer to the device installation function
	/* imagesensor operation functions: */
	int (*enable)(struct imagesensor *sensor);
	void (*disable)(struct imagesensor *sensor);
	int (*get_config)(struct imagesensor *sensor, struct scanunit_config *config);
	int (*set_config)(struct imagesensor *sensor, const struct scanunit_config *config);
};

/* declaration of imagesensor list */
extern struct imagesensor imagesensor_list[];
extern const int imagesensor_num;


/* function prototypes of image sensor driver (user level) */
extern int imagesensor_install_devices(void);

extern struct imagesensor *imagesensor_get(int index);

extern int imagesensor_enable(struct imagesensor *sensor);
extern void imagesensor_disable(struct imagesensor *sensor);

extern int imagesensor_get_config(struct imagesensor *sensor, struct scanunit_config *config);
extern int imagesensor_set_config(struct imagesensor *sensor, const struct scanunit_config *config);


#endif /* __IMAGESENSOR_H__ */
