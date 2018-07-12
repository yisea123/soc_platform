#ifndef __FPGA_CIS_H__
#define __FPGA_CIS_H__

#include "imagesensor.h"


struct fpga_cis_resource {
	void *ctrl_base;
	void *mmio_base;
	uint32_t mask;
};

extern int fpga_cis_install(struct imagesensor *sensor);
extern int fpga_cis_drvinit(void);

#endif