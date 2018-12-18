#ifndef __NVRAM_H__
#define __NVRAM_H__

#include <stdint.h>
#include <stddef.h>

typedef enum{
	NVRAM_TYPE_ENVM,
	NVRAM_TYPE_EEPROM,
	NVRAM_TYPE_SPIFLASH,
}nvram_type_t;

/* nvram feature information block */
struct nvram_feature {
	nvram_type_t type;		// nvram type
	int size;			// size of nvram device (in Bytes)
	int pagesize;			// size of a nvram device page (in Bytes)
};


/*
 * struct nvram
 *
 * One for each nvram device.
 */
struct nvram {
	const void *resource;				// pointer to the resource block of a nvram instance
	int (*const install)(struct nvram *nvm);	// pointer to the device installation function 
	struct nvram_feature feature;
	/* nvram operation functions: */
	int (* global_unprotect)( struct nvram *nvm );
	int (* erase)(struct nvram *nvm);
	int (* erase_block)(struct nvram *nvm, uint32_t address, uint32_t block_size);
	int (* read)(struct nvram *nvm, void *buffer, int offset, int count);
	int (* write)(struct nvram *nvm, const void *buffer, int offset, int count);
};


/* declaration of nvram list */
extern struct nvram nvram_list[];
extern const int nvram_num;

/* definition of inline function of nvram driver */
static inline const struct nvram_feature *nvram_get_feature(struct nvram *nvm)
{
	return (!nvm) ? NULL : &nvm->feature;
}

/* function prototypes of nvram driver */
int nvram_install_devices(void);

struct nvram *nvram_get(int index);

int nvram_erase(struct nvram *nvm);
int nvram_read(struct nvram *nvm, void *buffer, int offset, int count);
int nvram_write(struct nvram *nvm, const void *buffer, int offset, int count);

#endif /* __NVRAM_H__ */
