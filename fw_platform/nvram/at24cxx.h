#ifndef __AT24CXX_H__
#define __AT24CXX_H__

#include "nvram.h"
#include "mss_i2c.h"


struct at24cxx_resource {
  	// resource of AT24Cxx EEPROM nvram
	mss_i2c_instance_t *i2c;
	int size;		// size of AT24Cxx eeprom chip
	uint8_t i2c_address;	// i2c  SLAVE ADDRESS
	uint16_t pagesize;
};

extern int at24cxx_install(struct nvram *nvm);
extern int at24cxx_drvinit(void);

#endif
