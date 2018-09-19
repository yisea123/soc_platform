#ifndef __USBDRV_H__
#define __USBDRV_H__

#include <stdint.h>


/* USB device configuration block */
struct usbd_config
{
	uint16_t vendor_id;		/* USB Vendor ID */
	uint16_t product_id;		/* USB Product ID */
	uint8_t	usb_class;		/* USB class */
	const char *product_name;	/* product name */
};


int usbd_install(const struct usbd_config *config);
int usbd_read(uint8_t* buf, uint32_t len);
int usbd_write(const uint8_t* buf, uint32_t len);

#endif /* __USBDRV_H__ */
