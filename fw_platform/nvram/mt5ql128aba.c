#include "spiflashdev.h"


spiflash_info_t spiflash_info_MT25QL128ABA = {
	.spiflash_type = SPIFLASH_MT25QL128ABA,	
	.spiflash_id={
	/* Manufacture and device IDs for Spansion Electronics MT25QL128ABA SPI Flash. */
		.manufacturer_id = 0x20,
		.device_id = {0xBA, 0x18},
		.device_id_num = 2, 
		.extern_id_num = 0
	},
	.nvram_feature_struct = {
		.type = NVRAM_TYPE_SPIFLASH,		// nvram type
		.size = 16*1024*1024,			// size of nvram device (in Bytes)
		.pagesize = SECTOR_SIZE_64KB		// size of a nvram device page (in Bytes)
	}
};

