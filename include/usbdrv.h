#ifndef __USBDRV_H__
#define __USBDRV_H__

#include <stdint.h>

/* USB device configuration block */
struct usbd_config
{
	uint16_t vendor_id;		/* USB Vendor ID */
	uint16_t product_id;		/* USB Product ID */
	uint16_t bcd_device;		/* USB bcd_device */
	const char *str_manufacture;	/* manufacture string */
	const char *str_product;	/* product string */
	const char *str_serial;		/* serial string */
	const char *str_config;		/* config string */
	const char *str_interface;	/* interface string */
};


typedef enum {
	IDVENDOR_LSB_IDX = 8,
	IDVENDOR_MSB_IDX,
	IDPRODUCT_LSB_IDX,
	IDPRODUCT_MSB_IDX,
	BCDDEVICE_LSB_IDX,
	BCDDEVICE_MSB_IDX,
}usbd_descriptor_idx_t;


int usbd_install(const struct usbd_config *config);
int usbd_read(uint8_t* buf, uint32_t len);
int usbd_write(const uint8_t* buf, uint32_t len);

#endif /* __USBDRV_H__ */
