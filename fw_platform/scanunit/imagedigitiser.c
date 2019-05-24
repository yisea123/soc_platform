/*
 * general image digitiser/Analog Front End (AFE) functions implementation
 *
 * Copyright 2018 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "imagedigitiser.h"


int imagedigitiser_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<imagedigitiser_num; i++)
	{
		if (imagedigitiser_list[i]->install != NULL)
		{
			rs = imagedigitiser_list[i]->install(imagedigitiser_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct imagedigitiser *imagedigitiser_get(int index)
{
	if (index >= imagedigitiser_num || index < 0)
		return NULL;
	else
		return imagedigitiser_list[index];
}


int imagedigitiser_enable(struct imagedigitiser *afe)
{
	if (afe)
		return afe->enable(afe);

	return -1;
}


void imagedigitiser_disable(struct imagedigitiser *afe)
{
	if (afe)
		afe->disable(afe);
}


int imagedigitiser_get_config(struct imagedigitiser *afe, struct scanunit_config *config)
{
	if (afe)
		return afe->get_config(afe, config);

	return -1;
}


int imagedigitiser_set_config(struct imagedigitiser *afe, const struct scanunit_config *config)
{
	if (afe)
		return afe->set_config(afe, config);

	return -1;
}


int imagedigitiser_get_aux_config(struct imagedigitiser *afe, struct scanunit_config *config)
{
	if (afe)
		return afe->get_aux_config(afe, config);

	return -1;
}


int imagedigitiser_set_aux_config(struct imagedigitiser *afe, const struct scanunit_config *config)
{
	if (afe)
		return afe->set_aux_config(afe, config);

	return -1;
}
