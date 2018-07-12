/*
 * image digitiser (analog front end [AFE]) device driver definitions
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __IMAGEDIGITISER_H__
#define __IMAGEDIGITISER_H__

#include <stddef.h>
#include <stdint.h>
#include "scanunit.h"


/*
 * struct imagedigitiser
 *
 * One for each image digitiser device.
 */
struct imagedigitiser {
	const void *resource;				// pointer to the resource block of an imagedigitiser instance
	int (*const install)(struct imagedigitiser *afe);	// pointer to the device installation function
	/* imagedigitiser operation functions: */
	int (*enable)(struct imagedigitiser *afe);
	void (*disable)(struct imagedigitiser *afe);
	int (*get_config)(struct imagedigitiser *afe, struct scanunit_config *config);
	int (*set_config)(struct imagedigitiser *afe, const struct scanunit_config *config);
	int (*get_aux_config)(struct imagedigitiser *afe, struct scanunit_config *config);
	int (*set_aux_config)(struct imagedigitiser *afe, const struct scanunit_config *config);
};


/* declaration of imagedigitiser list */
extern struct imagedigitiser imagedigitiser_list[];
extern const int imagedigitiser_num;

/* function prototypes of image digitiser driver (user level) */
extern int imagedigitiser_install_devices(void);

extern struct imagedigitiser * imagedigitiser_get(int index);

extern int imagedigitiser_enable(struct imagedigitiser *afe);
extern void imagedigitiser_disable(struct imagedigitiser *afe);

extern int imagedigitiser_get_config(struct imagedigitiser *afe, struct scanunit_config *config);
extern int imagedigitiser_set_config(struct imagedigitiser *afe, const struct scanunit_config *config);

extern int imagedigitiser_get_aux_config(struct imagedigitiser *afe, struct scanunit_config *config);
extern int imagedigitiser_set_aux_config(struct imagedigitiser *afe, const struct scanunit_config *config);

#endif /* __IMAGEDIGITISER_H__ */
