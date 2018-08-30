/*
 * SmartFusion2 SOC eNVM nvram driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */

#include <stdint.h>
#include <string.h>

#include "nvram.h"
#include "socnvm.h"
#include "mss_nvm.h"


static int socnvm_erase(struct nvram *nvm)
{
	return 0;
}


static int socnvm_read(struct nvram *nvm, void *buffer, int offset, int count)
{
	struct socnvm_resource *nvm_rc;
	uint8_t *nvm_addr;
	int cnt;

	nvm_rc = (struct socnvm_resource *)nvm->resource;
	cnt = (count + offset <= nvm_rc->size) ? count : (nvm_rc->size - offset);
	nvm_addr = (uint8_t *)nvm_rc->start_addr + offset;

	memcpy(buffer, nvm_addr, cnt);

	return cnt;
}


static int socnvm_write(struct nvram *nvm, const void *buffer, int offset, int count)
{
	struct socnvm_resource *nvm_rc;
	uint32_t nvm_addr;
	int cnt, rs;

	nvm_rc = (struct socnvm_resource *)nvm->resource;
	cnt = (count + offset <= nvm_rc->size) ? count : (nvm_rc->size - offset);
	nvm_addr = (uint32_t)nvm_rc->start_addr + offset;

	rs = NVM_write(nvm_addr, (const uint8_t *)buffer, (uint32_t)cnt, NVM_DO_NOT_LOCK_PAGE);
	if (rs == NVM_SUCCESS)
		return cnt;
	else
		return -1;
}


int socnvm_install(struct nvram *nvm)
{
	struct socnvm_resource *nvm_rc;

	if (!nvm)
		return -1;
	if (!nvm->resource)
		return -1;

	nvm_rc = (struct socnvm_resource *)nvm->resource;
	// set nvram features
	nvm->feature.size = nvm_rc->size;
	nvm->feature.pagesize = 128;

	// set nvram operation functions
	nvm->erase = socnvm_erase;
	nvm->read = socnvm_read;
	nvm->write = socnvm_write;

	return 0;
}


int socnvm_drvinit(void)
{
	return 0;
}
