/*
 * image scanning unit device driver user interface definitions
 *
 * Copyright 2016 Hunan GreatWall Information Financial Equipment Co., Ltd.
 *
 */
#ifndef __SCANUNIT_H__
#define __SCANUNIT_H__



#define MAX_IMAGE_SENSORS	2
#define MAX_IMAGE_DIGITISERS	8


/* scanning section descrtion block */
struct scan_section {
	int lstart;				// start logical pixel nubmer of a scanning section
	int lend;				// end logical pixel nubmer of a scanning section
	int pstart;				// start physical pixel nubmer of a scanning section
	int pend;				// end physical pixel nubmer of a scanning section
	int digitiser_id;			// ID of the digitiser which converts pixel data of a section
	int channel;				// channel of the digitiser which converts pixel data of a section
};


/* scanunit hardware information block */
struct scanunit_hwinfo {
	int sides;				// number of scanning side
	int colors;				// image sensor colors (1 - greyscale; 3 - R/G/B color)
	int lightsources;			// number of light sources
	int sensors;				// number of image sensor
	int digitisers;				// number of image digitiser
	int sensor_a;				// image sensor ID of side A
	int sections_a;				// number of scanning sections of side A
	struct scan_section sectinfo_a[10];	// scanning section information block of side A	
	int sensor_b;				// image sensor ID of side B
	int sections_b;				// number of scanning sections of side B
	struct scan_section sectinfo_b[10];	// scanning section information block of side B
};

#define MAX_SCANNING_SECTIONS	(sizeof(((struct scanunit_hwinfo *)0)->sectinfo_a)/sizeof(struct scan_section))


/* scanunit hardware register configuration structure */
struct scan_reg_config {
	unsigned int address;			// address of scanunit register to configure
	unsigned int value;			// value of scanunit register to configure
	unsigned int mask;			// bit mask when mask is not ZERO
};


/* scanunit hardware configuration block */
struct scanunit_config {
	int regcount;				// value of scanunit register to configure
	struct scan_reg_config *regconfig;	// address of scanunit register configure block
};

struct scanunit_scanmode {
	unsigned char dpimode;
	unsigned char ledmode;
};
#endif /* __SCANUNIT_H__ */
