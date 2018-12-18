#include "spiflashdev.h"


spiflash_info_t spiflash_info_S25FL128PIFL_sector256KB = {
	.spiflash_type = SPIFLASH_S25FL128PIFL_SECTOR256KB,
	.spiflash_id={
		/* Manufacture and device IDs for Spansion Electronics FL128PIFL SPI Flash. */
		.manufacturer_id = 0x01,
		.device_id = {0x20, 0x18},
		.device_id_num = 2,
		.extern_id = {0x03, 0x00},
		.extern_id_num = 2
	},
	.nvram_feature_struct = {
		.type = NVRAM_TYPE_SPIFLASH,		// nvram type
		.size = 16*1024*1024,			//16MB // size of nvram device (in Bytes)
		.pagesize = SECTOR_SIZE_256KB		//256KB // size of a nvram device page (in Bytes)
	}
};

spiflash_info_t spiflash_info_S25FL128PIFL_sector64KB = {
	.spiflash_type = SPIFLASH_S25FL128PIFL_SECTOR64KB,	
	.spiflash_id={
		/* Manufacture and device IDs for Spansion Electronics FL128PIFL SPI Flash. */
		.manufacturer_id = 0x01,
		.device_id = {0x20, 0x18},
		.device_id_num = 2,
		.extern_id = {0x03, 0x01},
		.extern_id_num = 2
	},
	.nvram_feature_struct = {
		.type = NVRAM_TYPE_SPIFLASH,		// nvram type
		.size = 16*1024*1024,			// size of nvram device (in Bytes)
		.pagesize = SECTOR_SIZE_64KB		// size of a nvram device page (in Bytes)
	}
};



