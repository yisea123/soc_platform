/***************************************************************************//**
 * (c) Copyright 2009 Actel Corporation.  All rights reserved.
 * 
 *  ST M25P128 SPI flash driver API.
 *
 * SVN $Revision:$
 * SVN $Date:$
 */

#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#include <stdint.h>
#include <stdlib.h>

#include "mss_spi.h"
#include "mss_pdma.h"
#include "nvram.h"
#include "spiflashdev.h"


// resource of spi flash
struct spiflash_resource {
  	mss_spi_instance_t *spi_instance;
	int spi_slave_id;
	spiflash_info_t *pspiflash_info;
};


extern int spiflash_install(struct nvram *nvm);
extern int spiflash_drvinit(void);

extern int spiflash_global_unprotect( struct nvram *nvm );
extern int spiflash_erase_sector(    struct nvram *nvm, uint32_t offset);
extern int spiflash_read(    struct nvram *nvm,  void *buffer, int offset, int size_in_bytes);
extern int spiflash_write(struct nvram *nvm, const void *buffer, int offset, int size_in_bytes);

#endif

