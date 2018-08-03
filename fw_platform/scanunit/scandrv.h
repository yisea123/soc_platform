/*
 * image scanning unit device driver user-space API library definition
 *
 * Copyright 2016 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __SCANDRV_H__
#define __SCANDRV_H__

#include <scanunit.h>

struct scanunit_resource {
	void *ctrl_base;
	uint32_t int_mask;
	int fabric_irq;
};


extern int scanunit_install(const struct scanunit_resource *);
extern void scanunit_reset(void);
extern void scanunit_start_scanning(void);
extern void scanunit_stop_scanning(void);
extern void scanunit_turnon_lights(void);
extern void scanunit_turnoff_lights(void);
extern void scanunit_set_scanning_mode(uint32_t mode);
extern int scanunit_get_hwinfo(const struct scanunit_hwinfo *hwinfo);
extern int scanunit_get_digitiser_config(int device, struct scanunit_config *config);
extern int scanunit_set_digitiser_config(int device, const struct scanunit_config *config);
extern int scanunit_get_sensor_config(int device, struct scanunit_config *config);
extern int scanunit_set_sensor_config(int device, const struct scanunit_config *config);
extern int scanunit_get_sensor_common_config(struct scan_reg_config *regconfig);
extern int scanunit_set_sensor_common_config(const struct scan_reg_config *regconfig);

#endif /* __SCANDRV_H__ */
