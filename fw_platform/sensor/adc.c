/*
 * Analog-to-Digital Converter (ADC) driver (device-independant abstraction layer)
 *
 * Copyright 2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#include "adc.h"


int adc_install_devices(void)
{
	int i, rs, dev_err;

	dev_err = 0;
	for (i=0; i<adconverter_num; i++)
	{
		if (ad_converter_list[i]->install != NULL)
		{
			rs = ad_converter_list[i]->install(ad_converter_list[i]);
			if (rs == 0)
				continue;
		}
		++dev_err;
	}
	return -dev_err;
}


struct ad_converter *adc_get(int index)
{
	if (index >= adconverter_num || index < 0)
		return NULL;
	else
		return ad_converter_list[index];
}


int adc_init(struct ad_converter *adc, void *data)
{
	if (adc || adc->channels > MAX_ADC_CHANNELS)
		return adc->init(adc, data);

	return -1;
}


int adc_status(struct ad_converter *adc, uint32_t *status)
{
	if (adc)
		return adc->status(adc, status);

	return -1;
}


void adc_enable(struct ad_converter *adc, int channel)
{
	if (adc)
		adc->enable(adc, channel);
}


void adc_disable(struct ad_converter *adc, int channel)
{
	if (adc)
		adc->disable(adc, channel);
}


int adc_read_raw(struct ad_converter *adc, int channel, uint32_t *value)
{
	if (!adc || channel < 0 || channel >= adc->channels)
		return -1;

	return adc->read_raw(adc, channel, value);
}


int adc_read_average(struct ad_converter *adc, int channel, uint32_t *value)
{
	if (!adc || channel < 0 || channel >= adc->channels)
		return -1;

	return adc->read_average(adc, channel, value);
}


int adc_read_compare(struct ad_converter *adc, int channel, uint32_t *value)
{
	if (!adc || channel < 0 || channel >= adc->channels)
		return -1;

	return adc->read_compare(adc, channel, value);
}


int adc_set_threshold(struct ad_converter *adc, int channel, uint32_t value)
{
	if (!adc || channel < 0 || channel >= adc->channels)
		return -1;

	return adc->set_threshold(adc, channel, value);
}


int adc_set_event(struct ad_converter *adc, int channel, adc_event_t event)
{
	if (!adc || channel < 0 || channel >= adc->channels)
		return -1;

	adc->event[channel] = event;
	return adc->set_event(adc, channel, event);
}


int adc_unset_event(struct ad_converter *adc, int channel)
{
	if (!adc || channel < 0 || channel >= adc->channels)
		return -1;

	adc->event[channel] = ADC_EVT_NONE;
	return adc->unset_event(adc, channel);
}


int adc_set_callback(struct ad_converter *adc, int channel, void (*callback)(void *, int, void *), void *data)
{
	if (!adc || channel < 0 || channel >= adc->channels)
		return -1;

	adc->callback[channel] = callback;
	adc->callbackdata[channel] = data;

	return 0;
}
