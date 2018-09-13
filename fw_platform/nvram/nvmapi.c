#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nvram.h"
#include "nvmapi.h"


#define DEBUG	1
#undef DEBUG


static nvram_control_t *nvmsect_table = NULL;
static int nvmsect_num = 0;

extern uint32_t crc32( const unsigned char *buf, uint32_t size);

static int calc_offset(nvram_block_t *block_table, int block_num);

static int id_to_index(int id);

// macros to get section & block index from return value of function id_to_index.
// return value: higher 16-bits = section index, lower 16-bits = block index
#define RETVAL_INDEX(sect, blk)	((sect) << 16 + (blk))
#define SECTION_INDEX(v)	((v) >> 16)
#define BLOCK_INDEX(v)		((v) & 0xffff)


int calc_offset(nvram_block_t *block_table, int block_num)
{
	int i;

	block_table[0].offset = 0;
	for(i = 1; i < block_num; i++)
		block_table[i].offset = block_table[i - 1].offset + block_table[i - 1].info->length;

	return block_table[i-1].offset + block_table[i-1].info->length;
}


/* block ID to section and block index */
int id_to_index(int id)
{
	int i, s;
	for (s = 0; s < nvmsect_num; s++)
		for (i = 0; i < nvmsect_table[s].blocks; i++)
			if (nvmsect_table[s].nvmblock[i].id == id)
				return RETVAL_INDEX(s, i);
	return -1;
}


/*
 *  nvram section register
 */
int nvm_register(int sections, nvram_control_t *nvm_sect)
{

	if (sections < 0 || nvm_sect == NULL)
		return -1;

	nvmsect_table = nvm_sect;
	nvmsect_num = sections;

	return 0;
}


/*
 *  nvram management initialize
 */
int nvm_initialize(int section)
{
	nvram_control_t *nvmsect;
	nvram_data_t *nvmdata;
	struct nvram *nvm;
	uint8_t *buffer;
	int count;
	int rs;

	if (!nvmsect_table || section > nvmsect_num)
		return -1;

	nvmsect = &nvmsect_table[section];
	nvmsect->nvm = NULL;
	nvm = nvram_get(nvmsect->dev_id);
	if (!nvm)
		return -1;

	nvmsect->nvm = nvm;

	count = calc_offset(nvmsect->nvmblock, nvmsect->blocks);
	nvmsect->datacnt = count;
	count += sizeof(nvram_data_t);	/* add nvram data head */
	if (count > nvmsect->length)
		return -1;

	buffer = malloc(count);
	if (!buffer)
		return -1;

	nvmsect->buflen = count;
	nvmsect->buffer = buffer;
	nvram_read(nvm, buffer, nvmsect->offset, nvmsect->buflen);

	rs = 0;
	nvmdata = (nvram_data_t *)buffer;
	if (nvmdata->validflag != nvmsect->validflag)
		rs = NVM_ERR_VALIDFLAG;
	else if (nvmdata->version != nvmsect->version)
		rs = NVM_ERR_VERSION;
	else if (nvmdata->length != nvmsect->datacnt)
		rs = NVM_ERR_LENGTH;
	else if (nvmdata->checksum != crc32(nvmdata->data, nvmdata->length))
		rs = NVM_ERR_CHECKSUM;
	if (rs != 0)
	{
		nvmdata->validflag = nvmsect->validflag;
		nvmdata->version = nvmsect->version;
		nvmdata->length = nvmsect->datacnt;
		memset(nvmdata->data, 0xff, nvmdata->length);	// set data area to blank data 0xFF

		nvmsect->flags = NVM_FLAG_DATA_INVALID;
	}
	else
		nvmsect->flags = NVM_FLAG_NORMAL;

	return rs;
}


/*
 *  nvram management resource release
 */
void nvm_release(void)
{
	int i;
	for (i = 0; i < nvmsect_num; i++)
	{
		free(nvmsect_table[i].buffer);
		nvmsect_table[i].buffer = NULL;
	}
}


/*
 *  set default value of a nvram data section
 */
int nvm_setdefault(int section)
{
	nvram_control_t *nvmsect;
	nvram_block_t *nvmblock;
	nvram_data_t *nvmdata;
	uint32_t chksum;
	int i;

	if (!nvmsect_table || section > nvmsect_num)
		return -1;

	nvmsect = &nvmsect_table[section];
	nvmblock = nvmsect->nvmblock;
	nvmdata = (nvram_data_t *)nvmsect->buffer;
	if (!nvmdata)
		return -1;

	for (i = 0; i < nvmsect->blocks; i++) {
		uint8_t *dataptr = &(nvmdata->data[nvmblock[i].offset]);
		if (nvmblock[i].info->defaultdata)
			memcpy(dataptr, nvmblock[i].info->defaultdata, nvmblock[i].info->length);
	}
	chksum = crc32(nvmdata->data, nvmdata->length);
	if (chksum != nvmdata->checksum) {
		nvmdata->checksum = chksum;
		nvram_write(nvmsect->nvm, nvmsect->buffer, nvmsect->offset, nvmsect->buflen);
	}

	return 0;
}


/*
 *  write data block to nvram
 */
int nvm_write(int block_id, void *buffer, int count)
{
	nvram_control_t *nvmsect;
	nvram_block_t *nvmblock;
	nvram_data_t *nvmdata;
	uint8_t *dataptr;
	int index, section, val;

	val = id_to_index(block_id);
	if (val == -1)
		return -1;

	section = SECTION_INDEX(val);
	index = BLOCK_INDEX(val);
	nvmsect = &nvmsect_table[section];

	nvmblock = &(nvmsect->nvmblock[index]);
	if (count > nvmblock->info->length)
		count = nvmblock->info->length;

	nvmdata = (nvram_data_t *)nvmsect->buffer;
	dataptr = &(nvmdata->data[nvmblock->offset]);
	if (memcmp(dataptr, buffer, count) != 0)
	{
		uint32_t chksum;
		memcpy(dataptr, buffer, count);
		chksum = crc32(nvmdata->data, nvmdata->length);
		if (chksum != nvmdata->checksum) {
			nvmdata->checksum = chksum;
			nvmsect->flags = NVM_FLAG_FLUSH_PENDING;
			// TODO: change to delayed nvram writing operation
			nvram_write(nvmsect->nvm, nvmsect->buffer, nvmsect->offset, nvmsect->buflen);
			nvmsect->flags &= ~NVM_FLAG_FLUSH_PENDING;
		}
	}
	return count;
}


/*
 *  read data block from nvram
 */
int nvm_read(int block_id, void *buffer, int count)
{
	nvram_control_t *nvmsect;
	nvram_block_t *nvmblock;
	nvram_data_t *nvmdata;
	uint8_t *dataptr;
	int index, section, val;

	val = id_to_index(block_id);
	if (val == -1)
		return -1;

	section = SECTION_INDEX(val);
	index = BLOCK_INDEX(val);
	nvmsect = &nvmsect_table[section];

	nvmblock = &(nvmsect->nvmblock[index]);
	if (count > nvmblock->info->length)
		count = nvmblock->info->length;

	nvmdata = (nvram_data_t *)nvmsect->buffer;
	dataptr = &(nvmdata->data[nvmblock->offset]);
	memcpy(buffer, dataptr, count);

	return count;
}


/*
 *  flush all pennding nvram data writing
 */
void nvm_flush(void)
{
	int i;
	for (i=0; i<nvmsect_num; i++) {
		nvram_control_t *nvmsect;
		nvmsect = &nvmsect_table[i];
		if (nvmsect_table[i].flags & NVM_FLAG_FLUSH_PENDING)
		{
			nvram_write(nvmsect->nvm, nvmsect->buffer, nvmsect->offset, nvmsect->buflen);
			nvmsect->flags &= ~NVM_FLAG_FLUSH_PENDING;
		}
	}
}

