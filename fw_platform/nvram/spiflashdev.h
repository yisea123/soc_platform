#ifndef __SPIFLASHDEV_H__
#define __SPIFLASHDEV_H__

#include <stdint.h>
#include <stdlib.h>
#include "nvram.h"

typedef enum { 
	SPIFLASH_S25FL128PIFL_SECTOR256KB,
	SPIFLASH_S25FL128PIFL_SECTOR64KB,
	SPIFLASH_MT25QL128ABA,
	SPIFLASH_W25Q16JV,
	SPIFLASH_W25Q32JV
}spiflash_type_t;

#define SECTOR_SIZE_256KB	0x40000	//256KB
#define SECTOR_SIZE_64KB	0x10000	//64KB

#pragma pack(1)

typedef struct{
	uint8_t  manufacturer_id;
	uint8_t  device_id[2];
	uint8_t  device_id_num;
	uint8_t  extern_id[2];
	uint8_t  extern_id_num;
}spiflash_id_t;

typedef struct{
	spiflash_type_t spiflash_type;
	spiflash_id_t	spiflash_id;
	struct nvram_feature  nvram_feature_struct;
}spiflash_info_t;
#pragma pack()

extern spiflash_info_t spiflash_info_S25FL128PIFL_sector256KB;
extern spiflash_info_t spiflash_info_S25FL128PIFL_sector64KB;
extern spiflash_info_t spiflash_info_MT25QL128ABA;
extern spiflash_info_t spiflash_info_W25Q16JV;
extern spiflash_info_t spiflash_info_W25Q32JV;


#endif
