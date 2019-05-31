/*
 * Analog-to-Digital Converter (ADC) device driver definitions
 *
 * Copyright 2019 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __ADC_H__
#define __ADC_H__

#include <stddef.h>
#include <stdint.h>


#define MAX_ADC_CHANNELS	24


/* generic ADC events */
typedef enum {
	ADC_EVT_NONE,
	ADC_EVT_CONVERSION_DONE,
	ADC_EVT_CHANGE_RISING,
	ADC_EVT_CHANGE_FALLING,
	ADC_EVT_CHANGE_EITHER,
} adc_event_t;


/*  ADC device control types () */
typedef enum {
	ADC_CTRL_SOFTWARE,	/* ADC device is controlled by software */
	ADC_CTRL_HARDWARE,	/* ADC device is controlled by hardware (typically FPGA logic) */
} adc_ctrl_t;


/*
 * struct ad_converter
 *
 * One for each ADC device.
 */
struct ad_converter {
	const void *resource;				// pointer to the resource block of an ADC instance
	int (*const install)(struct ad_converter *adc);	// pointer to the device installation function
	int channels;					// number of channels of an ADC device
	adc_ctrl_t ctrltype;				// ADC device control type

	/* ADC event callback definition */
	void (*callback[MAX_ADC_CHANNELS])(void *adc, int channel, void *data);	// callback handling ADC events
	adc_event_t event[MAX_ADC_CHANNELS];
	void *callbackdata[MAX_ADC_CHANNELS];

	/* ADC operation functions */
	int (*init)(struct ad_converter *adc, void *data);
	void (*enable)(struct ad_converter *adc, int channel);
	void (*disable)(struct ad_converter *adc, int channel);
	int (*status)(struct ad_converter *adc, uint32_t *status);
	int (*read_raw)(struct ad_converter *adc, int channel, uint32_t *value);
	int (*read_average)(struct ad_converter *adc, int channel, uint32_t *value);
	int (*read_compare)(struct ad_converter *adc, int channel, uint32_t *value);
	int (*set_threshold)(struct ad_converter *adc, int channel, uint32_t value);
	int (*set_event)(struct ad_converter *adc, int channel, adc_event_t event);
	int (*unset_event)(struct ad_converter *adc, int channel);
};


/* declaration of ADC list */
extern struct ad_converter * ad_converter_list[];
extern const int adconverter_num;


/* function prototypes of ADC driver */
extern struct ad_converter *adc_get(int index);
extern int adc_install_devices(void);

extern int adc_init(struct ad_converter *adc, void *data);
extern void adc_enable(struct ad_converter *adc, int channel);
extern void adc_disable(struct ad_converter *adc, int channel);
extern int adc_status(struct ad_converter *adc, uint32_t *status);
extern int adc_read_raw(struct ad_converter *adc, int channel, uint32_t *value);
extern int adc_read_average(struct ad_converter *adc, int channel, uint32_t *value);
extern int adc_read_compare(struct ad_converter *adc, int channel, uint32_t *value);
extern int adc_set_threshold(struct ad_converter *adc, int channel, uint32_t value);

extern int adc_set_event(struct ad_converter *adc, int channel, adc_event_t event);
extern int adc_unset_event(struct ad_converter *adc, int channel);
extern int adc_set_callback(struct ad_converter *adc, int channel, void (*callback)(void *, int, void *), void *data);


#endif /* __ADC_H__ */
