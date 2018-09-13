#ifndef __NVMAPI_H__
#define __NVMAPI_H__

#include<stdint.h>
#define MAX_NAME_LEN	256

#pragma pack(1)		// set one-byte alignment to save space in NVRAM

// data structure of nvram data
typedef struct nvram_data_s
{
	uint16_t validflag;	// validate flag
	uint16_t version;	// nvram data profile version
	int length;		// nvram data length
	uint32_t checksum;	// CRC-32 checksum
	uint8_t data[];		// application data
} nvram_data_t;

#pragma pack()		// restore default alignment


// data structure of nvram data information
typedef struct nvmdata_info_s
{
	const int length;
	const void *const defaultdata;
} nvmdata_info_t;


// data structure of nvram block table
typedef struct nvram_block_s
{
	int offset;			// offset in nvram application data
	const nvmdata_info_t *const info; 	// pointer to nvram data information
	const int id;			// nvram block id
	const char *name;		// nvram block name
} nvram_block_t;


// data structure of nvram control block
typedef struct nvram_control_s
{
	const int dev_id;		// nvram device ID
	const uint16_t validflag;	// validate flag
	const uint16_t version;		// nvram data profile version
	const int offset;		// memory region offset in the nvram device
	const int length;		// memory region length
	const int blocks;		// number of data blocks in the nvram section
	nvram_block_t *const nvmblock;	// pointer to nvram block table
	int datacnt;			// nvram user data count
	int flags;			// nvram management flags
	struct nvram *nvm;		// pointer to nvram device
	unsigned char *buffer;		// pointer to RAM copy buffer of nvram data block
	int buflen;			// length of RAM copy buffer
} nvram_control_t;


/* nvram flags definition */
#define NVM_FLAG_NORMAL		0
#define NVM_FLAG_DATA_INVALID	0x01
#define NVM_FLAG_FLUSH_PENDING	0x02


/* nvram data errors definition */
#define NVM_ERR_VALIDFLAG	-11
#define NVM_ERR_VERSION		-12
#define NVM_ERR_LENGTH		-13
#define NVM_ERR_CHECKSUM	-14


/* macros of reference to nvram data ID, name, information variable */
#define NVM_DATA_NAME(name)	#name
#define NVM_DATA_ID(name)	NVMDATA_##name
#define NVM_DATA_INFO(name)	nvmdata_info_##name

/* macro to define and initialize a nvram data information variable */
#define DEFINE_NVM_DATA(name, len, defval)	const nvmdata_info_t NVM_DATA_INFO(name) = {(len), (defval)}

/* macro to declare a nvram data information variable */
#define DECLARE_NVM_DATA(name)			extern const nvmdata_info_t NVM_DATA_INFO(name)


/* function prototypes */
int nvm_register(int sections, nvram_control_t *nvm_sect);
int nvm_initialize(int section);
void nvm_release(void);
void nvm_flush(void);
int nvm_setdefault(int section);
int nvm_write(int block_id, void *buffer, int count);
int nvm_read(int block_id, void *buffer, int count);

#endif /* __NVMAPI_H__ */
