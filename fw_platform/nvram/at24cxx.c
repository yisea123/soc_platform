/*
 * AT24Cxx I2C EEPROM nvram driver
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */

#include <stdint.h>
#include <string.h>

#include "nvram.h"
#include "at24cxx.h"
#include "mss_i2c.h"


static int at24cxx_erase(struct nvram *nvm)
{
	return 0;
}


static int at24cxx_read(struct nvram *nvm, void *buffer, int offset, int count)
{
	struct at24cxx_resource *nvm_rc;
	int cnt;

	nvm_rc = (struct at24cxx_resource *)nvm->resource;
	cnt = (count + offset <= nvm_rc->size) ? count : (nvm_rc->size - offset);

	return cnt;
}


static int at24cxx_write(struct nvram *nvm, const void *buffer, int offset, int count)
{
	struct at24cxx_resource *nvm_rc;
	int cnt;

	nvm_rc = (struct at24cxx_resource *)nvm->resource;
	cnt = (count + offset <= nvm_rc->size) ? count : (nvm_rc->size - offset);

	return cnt;
}


int at24cxx_install(struct nvram *nvm)
{
	struct at24cxx_resource *nvm_rc;

	if (!nvm)
		return -1;
	if (!nvm->resource)
		return -1;

	nvm_rc = (struct at24cxx_resource *)nvm->resource;
	// set nvram features
	nvm->feature.size = nvm_rc->size;
	nvm->feature.pagesize = 1;

	// set nvram operation functions
	nvm->erase = at24cxx_erase;
	nvm->read = at24cxx_read;
	nvm->write = at24cxx_write;

	return 0;
}


int at24cxx_drvinit(void)
{
	return 0;
}

