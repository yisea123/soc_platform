/*
 * generic nvram driver functions implementation
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "nvram.h"


int nvram_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<nvram_num; i++)
	{
		if (nvram_list[i].install != NULL)
		{
			rs = nvram_list[i].install(&nvram_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct nvram *nvram_get(int index)
{
	if (index >= nvram_num || index < 0)
		return NULL;
	else
		return &nvram_list[index];
}


int nvram_erase(struct nvram *nvm)
{
	if (nvm)
		return nvm->erase(nvm);

	return -1;
}


int nvram_read(struct nvram *nvm, void *buffer, int offset, int count)
{
	if (nvm && buffer && count > 0 && offset < nvm->feature.size)
		return nvm->read(nvm, buffer, offset, count);

	return -1;
}


int nvram_write(struct nvram *nvm, const void *buffer, int offset, int count)
{
	if (nvm && buffer && count > 0 && offset < nvm->feature.size)
		return nvm->write(nvm, buffer, offset, count);

	return -1;
}

