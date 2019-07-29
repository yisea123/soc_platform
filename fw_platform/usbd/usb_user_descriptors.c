/*******************************************************************************
 * (c) Copyright 2012-2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * USB Vendor class Device example aplication to demonstrate the
 * SmartFusion2 MSS USB operations in USB Device mode.
 *
 * This file provides the Device Descriptor for the implemented USB Device.
 * This file implements Application call-back Interface structure type provided
 * by USBD driver.
 *
 * SVN $Revision: 7857 $
 * SVN $Date: 2015-09-23 18:11:40 +0530 (Wed, 23 Sep 2015) $
 */

#include "mss_usb_device.h"
#include "mss_usb_std_def.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "usbdrv.h"
/*******************************************************************************
 * Constant definitions
 */
#define  USB_MAX_STRING_DESCRIPTOR_SIZE                     64u

/* String Descriptor Indexes */
#define  USB_STRING_DESCRIPTOR_IDX_LANGID                   0x00
#define  USB_STRING_DESCRIPTOR_IDX_MANUFACTURER             0x01
#define  USB_STRING_DESCRIPTOR_IDX_PRODUCT                  0x02
#define  USB_STRING_DESCRIPTOR_IDX_SERIAL                   0x03
#define  USB_STRING_DESCRIPTOR_IDX_CONFIG                   0x04
#define  USB_STRING_DESCRIPTOR_IDX_INTERFACE                0x05

/*******************************************************************************
 * Local functions.
 */
uint8_t* vendor_dev_device_descriptor(uint32_t* length);
uint8_t* vendor_dev_device_qual_descriptor(mss_usb_device_speed_t speed,
                                            uint32_t* length);
uint8_t* vendor_dev_string_descriptor(uint8_t index, uint32_t* length);
uint8_t  vendor_dev_get_string(uint8_t* string, uint8_t* dest);

/***************************************************************************//**
  Device descriptor.
 */
uint8_t device_descriptor[USB_STD_DEVICE_DESCR_LEN] =
{
    USB_STD_DEVICE_DESCR_LEN,                           /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE,                         /* bDescriptorType */
    0x00u,                                              /* bcdUSB LSB */
    0x02u,                                              /* bcdUSB MSB */
    0xFFu,                                              /* bDeviceClass */
    0x00u,                                              /* bDeviceSubClass */
    0x00u,                                              /* bDeviceProtocol */
    0x40u,                                              /* bMaxPacketSize0 */
    0xBEu,                                              /* idVendor LSB */
    0x1Cu,                                              /* idVendor MSB */
    0x03u,                                              /* idProduct LSB */
    0x00u,                                              /* idProduct MSB */
    0x02u,                                              /* bcdDevice LSB */
    0x00u,                                              /* bcdDevice MSB */
    USB_STRING_DESCRIPTOR_IDX_MANUFACTURER,             /* iManufacturer */
    USB_STRING_DESCRIPTOR_IDX_PRODUCT,                  /* iProduct */
    USB_STRING_DESCRIPTOR_IDX_SERIAL,                   /* iSerialNumber */
    0x01u                                               /* bNumConfigurations */
};

/***************************************************************************//**
  Device qualifiers.
 */
uint8_t hs_dev_qualifier_descriptor[USB_STD_DEV_QUAL_DESCR_LENGTH] =
{
    USB_STD_DEV_QUAL_DESCR_LENGTH,                      /* bLength */
    USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE,               /* bDescriptorType */
    0x00u,                                              /* bcdUSB LSB */
    0x02u,                                              /* bcdUSB MSB */
    0xFFu,                                              /* bDeviceClass */
    0x00u,                                              /* bDeviceSubClass */
    0x00u,                                              /* bDeviceProtocol */
    0x40u,                                              /* bMaxPacketSize0 */
    0x01u,                                              /* bNumConfigurations */
    0x00u                                               /* Reserved */
};

uint8_t fs_dev_qualifier_descriptor[USB_STD_DEV_QUAL_DESCR_LENGTH] =
{
    USB_STD_DEV_QUAL_DESCR_LENGTH,                      /* bLength */
    USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE,               /* bDescriptorType */
    0x00u,                                              /* bcdUSB LSB */
    0x02u,                                              /* bcdUSB MSB */
    0xFFu,                                              /* bDeviceClass */
    0x00u,                                              /* bDeviceSubClass */
    0x00u,                                              /* bDeviceProtocol */
    0x40u,                                              /* bMaxPacketSize0 */
    0x01u,                                              /* bNumConfigurations */
    0x00u                                               /* Reserved */
};

uint8_t lang_string_descriptor[] =
{
    0x04u,                                              /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,                         /* bDescriptorType */
    0x09u,                                              /* LangID-LSB */
    0x04u                                               /* LangID-MSB */
};

mss_usbd_user_descr_cb_t vendor_dev_descriptors_cb = {
    vendor_dev_device_descriptor,
    vendor_dev_device_qual_descriptor,
    vendor_dev_string_descriptor,
};

uint8_t g_string_descriptor[USB_MAX_STRING_DESCRIPTOR_SIZE];
struct usbd_config *pusbd_config;

uint8_t*
vendor_dev_device_descriptor
(
    uint32_t* length
)
{
   *length = sizeof(device_descriptor);
   return(device_descriptor);
}

uint8_t*
vendor_dev_device_qual_descriptor
(
    mss_usb_device_speed_t speed,
    uint32_t* length
)
{
    if(speed == MSS_USB_DEVICE_HS)
    {
        *length = sizeof(fs_dev_qualifier_descriptor);
         return(fs_dev_qualifier_descriptor);
    }
    else
    {
        *length = sizeof(hs_dev_qualifier_descriptor);
         return(hs_dev_qualifier_descriptor);
    }
}

uint8_t*
vendor_dev_string_descriptor
(
    uint8_t index,
    uint32_t* length
)
{
    switch(index)
    {
        case USB_STRING_DESCRIPTOR_IDX_LANGID:
                *length = sizeof(lang_string_descriptor);
        break;

        case USB_STRING_DESCRIPTOR_IDX_MANUFACTURER:
		if(pusbd_config)
			*length = vendor_dev_get_string((uint8_t*)pusbd_config->str_manufacture,
		                             g_string_descriptor);
		else
			*length = 0;
		break;
        case USB_STRING_DESCRIPTOR_IDX_PRODUCT:
		if(pusbd_config)
			*length = vendor_dev_get_string((uint8_t*)pusbd_config->str_product,
		                             g_string_descriptor);
		else
			*length = 0;
        	break;

        case USB_STRING_DESCRIPTOR_IDX_SERIAL:
		if(pusbd_config)
			*length = vendor_dev_get_string((uint8_t*)pusbd_config->str_serial,
		                             g_string_descriptor);

		else
		     	*length = 0;
		break;
        case USB_STRING_DESCRIPTOR_IDX_CONFIG:
		if(pusbd_config)
			*length = vendor_dev_get_string((uint8_t*)pusbd_config->str_config,
                                             g_string_descriptor);
		else
		     	*length = 0;
       		break;
        case USB_STRING_DESCRIPTOR_IDX_INTERFACE:
		if(pusbd_config)
	    		*length = vendor_dev_get_string((uint8_t*)pusbd_config->str_interface,
                                             g_string_descriptor);
		else
		     	*length = 0;
        	break;
        default:
           /*Raise error*/
          *length = 0;
        break;
    }

    if(USB_STRING_DESCRIPTOR_IDX_LANGID == index)
    {
        return(lang_string_descriptor);
    }
    {
        return(g_string_descriptor);
    }
}

uint8_t
vendor_dev_get_string
(
    uint8_t* string,
    uint8_t* dest
)
{
    const uint8_t *idx = string ;
    uint8_t *cp_dest;

    cp_dest = dest;

    if((dest != 0u) && (string != 0u))
    {
        for (; *(idx); ++idx)
        {
            *(dest + 2u) = *(idx);
            dest++;
            *(dest + 2u) = 0x00u;
            dest++;
        }
        *cp_dest = ((idx - string) * 2u) + 2u;                /*bLength*/
        *(cp_dest + 1u) = USB_STRING_DESCRIPTOR_TYPE;        /*bDesriptorType*/
    }

    return(((idx - string) * 2u) + 2u);
}

#ifdef __cplusplus
}
#endif
