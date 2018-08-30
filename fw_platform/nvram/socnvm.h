#ifndef __SOCNVM_H__
#define __SOCNVM_H__

#include "nvram.h"


struct socnvm_resource {
  	// resource of SOC eNVM nvram
	void *start_addr;	// start address of nvram memory region in SOC eNVM 
	int size;		// size of nvram memory region in SOC eNVM
};

extern int socnvm_install(struct nvram *nvm);
extern int socnvm_drvinit(void);

#endif
