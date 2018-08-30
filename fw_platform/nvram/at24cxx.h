#ifndef __AT24CXX_H__
#define __AT24CXX_H__

#include "nvram.h"


struct at24cxx_resource {
  	// resource of AT24Cxx EEPROM nvram
	int size;		// size of nvram memory region in SOC eNVM
};

extern int at24cxx_install(struct nvram *nvm);
extern int at24cxx_drvinit(void);

#endif
