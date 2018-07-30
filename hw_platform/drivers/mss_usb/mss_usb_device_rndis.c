/***************************************************************************//**
 * (c) Copyright 2012-2016 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBD-RNDIS class driver.
 *
 * USBD-RNDIS class driver implementation:
 * This file implements RNDIS class Functionality. The RNDIS protocol level
 * handling is it the next level up.
 * 
 *
 * SVN $Revision: 8207 $
 * SVN $Date: 2016-01-06 14:42:19 +0530 (Wed, 06 Jan 2016) $
 */

#include "mss_usb_device_rndis.h"
#include "mss_usb_device.h"
#include "mss_usb_std_def.h"
#include "../../CMSIS/mss_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED
/***************************************************************************//**
 * Private Functions
 */
static uint8_t* RNDIS_get_descriptor_cb(uint8_t recepient,
                                        uint8_t type,
                                        uint32_t* length,
                                        mss_usb_device_speed_t musb_speed);
static uint8_t RNDIS_init_cb(uint8_t cfgidx, mss_usb_device_speed_t musb_speed);
static uint8_t RNDIS_deinit_cb(uint8_t cfgidx);
static uint8_t RNDIS_process_request_cb(mss_usbd_setup_pkt_t* setup_pkt,
                                        uint8_t** buf_pp,
                                        uint32_t* length);
static uint8_t RNDIS_datain_cb(mss_usb_ep_num_t num, uint8_t status);

static uint8_t RNDIS_dataout_cb(mss_usb_ep_num_t num,
                                uint8_t status,
                                uint32_t rx_count);
static uint8_t RNDIS_cep_datain_cb(uint8_t status);
static uint8_t RNDIS_cep_dataout_cb(uint8_t status);

mss_usbd_rndis_state_t g_usbd_rndis_state = USBD_RNDIS_NOT_CONFIGURED;

mss_usbd_class_cb_t usbd_rndis_class_cb = {
    RNDIS_init_cb,
    RNDIS_deinit_cb,
    RNDIS_get_descriptor_cb,
    RNDIS_process_request_cb,
    RNDIS_datain_cb,
    RNDIS_dataout_cb,
    RNDIS_cep_datain_cb,
    RNDIS_cep_dataout_cb
};

mss_usbd_rndis_app_cb_t* usbd_rndis_ops = 0;
mss_usb_device_speed_t g_usbd_rndis_user_speed;

/*Full configuration descriptor length. */
#define RNDIS_CONFIG_DESCR_LENGTH 70u

uint8_t rndis_conf_descr[RNDIS_CONFIG_DESCR_LENGTH] = {

    /*----------------------- Configuration Descriptor -----------------------*/
    USB_STD_CONFIG_DESCR_LEN,                  /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,         /* bDescriptorType */
    RNDIS_CONFIG_DESCR_LENGTH,                 /* wTotalLength LSB */
    0x00u,                                     /* wTotalLength MSB */
    0x02u,                                     /* bNumInterfaces */
                                               /* interface number (which starts
                                                  at 0) + 1 Comm Data Interface
                                                  + Comm Management interface.*/
    0x01u,                                     /* bConfigurationValue */
    0x00u,                                     /* iConfiguration */
    0xC0u,                                     /* bmAttributes - self powered*/
    0x32u,                                     /* bMaxPower */

    /*------------------- Interface Association Descriptor -------------------*/

    USB_STD_IA_DESCR_LEN,                      /* bLength*/
    USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE, /* bDescriptorType = 11u*/
    0x00,                                      /* bFirstInterface*/
    0x02,                                      /* bInterfaceCount*/
    0x02,                                      /* bFunctionClass (Communication
                                                  Class)*/
    0x02,                                      /* bFunctionSubClass (Abstract
                                                  Control Model)*/
    0xFF,                                      /* bFunctionProcotol Vendor
                                                  Specific Protocol*/
    0x00,                                      /* iInterface*/

    /*----------- Interface 0 - Communications management Descriptor ---------*/
    USB_STD_INTERFACE_DESCR_LEN,               /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,             /* bDescriptorType */
    0x00,                                      /* bInterfaceNumber: Number of
                                                  this Interface */
    0x00,                                      /* bAlternateSetting: Alternate
                                                  setting */
    0x01,                                      /* bNumEndpoints: one end point
                                                  used */
    0x02,                                      /* bInterfaceClass: CDC-Control*/
    0x02,                                      /* bInterfaceSubClass: Abstract
                                                  Control Model */
    0xFF,                                      /* bFunctionProcotol Vendor
                                                  Specific Protocol*/
    0x00,                                      /* iInterface: */

    /*------------------- Header Functional Descriptor -----------------------*/
    0x05,                                      /* bLength: Endpoint Descriptor
                                                  size */
    0x24,                                      /* bDescriptorType:
                                                  CS_INTERFACE */
    0x00,                                      /* bDescriptorSubtype: Header
                                                  Func Desc */
    0x10,                                      /* bcdCDC: spec release number */
    0x01,

    /*-----------------------ACM Functional Descriptor------------------------*/
    0x04,                                      /* bFunctionLength */
    0x24,                                      /* bDescriptorType:
                                                  CS_INTERFACE */
    0x02,                                      /* bDescriptorSubtype: Abstract
                                                  Control Management desc */
    0x00,                                      /* bmCapabilities */

    /*----------------------Union Functional Descriptor-----------------------*/
    0x05,                                      /* bFunctionLength */
    0x24,                                      /* bDescriptorType:
                                                  CS_INTERFACE */
    0x06,                                      /* bDescriptorSubtype: Union func
                                                  desc */
    0x00,                                      /* bMasterInterface:
                                                  Communication class
                                                  interface */
    0x01,                                      /* bSlaveInterface0: Data Class
                                                  Interface */

    /*----------- Communications management Endpoint Descriptor --------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,              /* bDescriptorType */
    0x82u,                                     /* bEndpointAddress */
    0x03u,                                     /* bmAttributes - INTERRUPT, not
                                                  synchronised, data transfer */
    0x08u,                                     /* wMaxPacketSize LSB */
    0x00u,                                     /* wMaxPacketSize MSB */
    0xFFu,                                     /* bInterval 0xFF equates to ? */

    /*-------------- Interface 1 - Data class interface descriptor -----------*/
    USB_STD_INTERFACE_DESCR_LEN,               /* bLength: Endpoint Descriptor
                                                  size */
    USB_INTERFACE_DESCRIPTOR_TYPE,             /* bDescriptorType: */
    0x01,                                      /* bInterfaceNumber: Number of
                                                  this Interface */
    0x00,                                      /* bAlternateSetting: Alternate
                                                  setting */
    0x02,                                      /* bNumEndpoints: Two endpoints
                                                  used */
    0x0A,                                      /* bInterfaceClass: CDC-Data */
    0x00,                                      /* bInterfaceSubClass: */
    0x00,                                      /* bInterfaceProtocol: */
    0x00,                                      /* iInterface: */

    /*-------------------------Endpoint OUT Descriptor------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                /* bLength: Endpoint Descriptor
                                                  size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,              /* bDescriptorType: Endpoint */
    0x01,                                      /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    0x40,                                      /* wMaxPacketSize: */
    0x00,
    0x05,                                      /* bInterval: ignored for Bulk
                                                  transfer */

    /*-------------------------Endpoint IN Descriptor-------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                /* bLength: Endpoint Descriptor
                                                  size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,              /* bDescriptorType: Endpoint */
    0x81,                                      /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    0x40,                                      /* wMaxPacketSize: */
    0x00,
    0x05                                       /* bInterval: ignored for Bulk
                                                  transfer */
};

/***************************************************************************//**
 * Exported functions from RNDIS class driver.
 */

/***************************************************************************//**
* See mss_usb_device_rndis.h for details of how to use this function.
*/
void MSS_USBD_RNDIS_init
(
    mss_usbd_rndis_app_cb_t* rndis_app_cb,
    mss_usb_device_speed_t speed
)
{
    usbd_rndis_ops = rndis_app_cb;
    g_usbd_rndis_user_speed = speed;
    MSS_USBD_set_class_cb_handler(&usbd_rndis_class_cb);
}

/***************************************************************************//**
* See mss_usb_device_rndis.h for details of how to use this function.
*/
void MSS_USBD_RNDIS_tx
(
    uint8_t* buf,
    uint32_t length
)
{
    MSS_USBD_tx_ep_write(RNDIS_BULK_IN_EP, buf, length);
    g_usbd_rndis_state = USBD_RNDIS_TRANSMITTING;
}

/***************************************************************************//**
 See mss_usb_device_rndis.h for details of how to use this function.
 */
void MSS_USBD_RNDIS_rx_prepare
(
    uint8_t* buf,
    uint32_t length
)
{
    MSS_USBD_rx_ep_read_prepare(RNDIS_BULK_OUT_EP, buf, length);
    g_usbd_rndis_state = USBD_RNDIS_RECEIVING;
}

/***************************************************************************//**
 See mss_usb_device_rndis.h for details of how to use this function.
 */
mss_usbd_rndis_state_t MSS_USBD_RNDIS_get_state
(
    void
)
{
    return g_usbd_rndis_state;
}

/***************************************************************************//**
 Private functions to USBD_RNDIS class driver.
 ******************************************************************************/
/***************************************************************************//**
* Returns config descriptor as appropriate for the interface speed.
*/
static uint8_t* RNDIS_get_descriptor_cb
(
    uint8_t recepient,
    uint8_t type,
    uint32_t* length,
    mss_usb_device_speed_t musb_speed
)
{
    /*User Selected FS:
            Operate only in FS
      User Selected HS:
        Device connected to 2.0 Host(musb_speed = HS):Operate in HS
        Device connected to 1.x Host(musb_speed = FS):Operate in FS
     */

    /*
     Since Endpoint Size is wMaxpacketSize is 64, which is valid for both
     FS and HS, no need to make decision based on musb_speed.
     */

    if(recepient == USB_STD_REQ_RECIPIENT_DEVICE )
    {
        if(type == USB_CONFIGURATION_DESCRIPTOR_TYPE)
        {
            rndis_conf_descr[1] = USB_CONFIGURATION_DESCRIPTOR_TYPE;
            *length = sizeof(rndis_conf_descr);
            return(rndis_conf_descr);
        }
        else if (type == USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE)
        {
            rndis_conf_descr[1] = USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE;
            *length = sizeof(rndis_conf_descr);
            return(rndis_conf_descr);
        }
    }
    else if(recepient == USB_STD_REQ_RECIPIENT_ENDPOINT)/*Need index(EP Num)*/
    {
        /*Do nothing*/
    }
    else if(recepient == USB_STD_REQ_RECIPIENT_INTERFACE)/*Need index(intrfs)*/
    {
        /*Do nothing*/
    }
    else
    {
        /*Do nothing*/
    }

    return USB_FAIL;
}

/***************************************************************************//**
* Set up Interrupt and Bulk End Point and initialize next level out.
*/
static uint8_t RNDIS_init_cb
(
    uint8_t cfgidx,
    mss_usb_device_speed_t musb_speed
)
{
    uint8_t *buf_p;
    uint32_t len;

    ASSERT(usbd_rndis_ops != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_init != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_notification != 0);

    /*
     Since Endpoint Size is wMaxpacketSize is 64, which is valid for both
     FS and HS, no need to make decision based on musb_speed.

     g_usbd_cdc_user_speed variable stores the speed selected by user.
     */
    MSS_USBD_tx_ep_configure(RNDIS_INTR_IN_EP,
                             RNDIS_INTR_IN_EP_FIFO_ADDR,
                             RNDIS_INTR_IN_EP_FIFO_SIZE,
                             RNDIS_INTR_IN_EP_MAX_PKT_SIZE,
                             1,
                             DMA_DISABLE,
                             MSS_USB_DMA_CHANNEL1,
                             MSS_USB_XFR_INTERRUPT,
                             ADD_ZLP_TO_XFR);

    MSS_USBD_tx_ep_configure(RNDIS_BULK_IN_EP,
                             RNDIS_BULK_IN_EP_FIFO_ADDR,
                             RNDIS_BULK_IN_EP_FIFO_SIZE,
                             RNDIS_BULK_IN_EP_MAX_PKT_SIZE,
                             1,
                             DMA_DISABLE,
                             MSS_USB_DMA_CHANNEL2,
                             MSS_USB_XFR_BULK,
                             ADD_ZLP_TO_XFR);

    MSS_USBD_rx_ep_configure(RNDIS_BULK_OUT_EP,
                             RNDIS_BULK_OUT_EP_FIFO_ADDR,
                             RNDIS_BULK_OUT_EP_FIFO_SIZE,
                             RNDIS_BULK_OUT_EP_MAX_PKT_SIZE,
                             1,
                             DMA_DISABLE,
                             MSS_USB_DMA_CHANNEL3,
                             MSS_USB_XFR_BULK,
                             ADD_ZLP_TO_XFR);
    g_usbd_rndis_state = USBD_RNDIS_CONFIGURED;
    usbd_rndis_ops->usb_rndis_init();

    /* Check for notifications in case outer level init has scheduled one */
    usbd_rndis_ops->usb_rndis_notification(&buf_p, &len);

    if(buf_p)
    {
        MSS_USBD_tx_ep_write(RNDIS_INTR_IN_EP, buf_p, len);
    }

    return 1;
}

/***************************************************************************//**
* Shut down the RNDIS interface
*/
static uint8_t RNDIS_deinit_cb
(
    uint8_t cfgidx
)
{
    ASSERT(usbd_rndis_ops != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_release != 0);

    usbd_rndis_ops->usb_rndis_release();
    g_usbd_rndis_state = USBD_RNDIS_NOT_CONFIGURED;

    return 1;
}

/***************************************************************************//**
* Process CDC level command
*/
static uint8_t RNDIS_process_request_cb
(
    mss_usbd_setup_pkt_t* setup_pkt,
    uint8_t** buf_pp,
    uint32_t* length
)
{
    ASSERT(usbd_rndis_ops != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_process_request != 0);

    return(usbd_rndis_ops->usb_rndis_process_request(setup_pkt,buf_pp,length));
}

/***************************************************************************//**
* Handle packet sent event on RNDIS Interrupt or Bulk End Points.
*/
static uint8_t RNDIS_datain_cb
(
    mss_usb_ep_num_t num,
    uint8_t status
)
{
    uint8_t *buf_p;
    uint32_t len;
    uint8_t result = USB_SUCCESS;

    ASSERT(usbd_rndis_ops != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_tx_complete != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_notification != 0);

    if(num == RNDIS_INTR_IN_EP)
    {
        usbd_rndis_ops->usb_rndis_notification(&buf_p, &len);
        if(buf_p)
        {
            MSS_USBD_tx_ep_write(RNDIS_INTR_IN_EP, buf_p, len);
        }
    }
    else if(num == RNDIS_BULK_IN_EP)
    {
        usbd_rndis_ops->usb_rndis_tx_complete(status);
    }
    else
    {
        result = USB_FAIL;
    }

    return(result);
}

/***************************************************************************//**
* Process incoming packet on RNDIS Bulk End Point
*/
static uint8_t RNDIS_dataout_cb
(
    mss_usb_ep_num_t num,
    uint8_t status,
    uint32_t rx_count
)
{
    ASSERT(usbd_rndis_ops != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_rx != 0);

    if(num == RNDIS_BULK_OUT_EP)
    {
        usbd_rndis_ops->usb_rndis_rx(status, rx_count);
    }
    else
    {
        return USB_FAIL;
    }
    return USB_SUCCESS;
}

/***************************************************************************//**
* Handle transmission of control packet complete for RNDIS EP 0
*/
static uint8_t RNDIS_cep_datain_cb
(
    uint8_t status
)
{
    ASSERT(usbd_rndis_ops != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_cep_tx_complete != 0);

    if(0 != usbd_rndis_ops->usb_rndis_cep_tx_complete)
        usbd_rndis_ops->usb_rndis_cep_tx_complete(status);
    return USB_SUCCESS;
}

/***************************************************************************//**
* Process incoming RNDIS control packet
*/
uint8_t RNDIS_cep_dataout_cb
(
    uint8_t status
)
{
    ASSERT(usbd_rndis_ops != 0);
    ASSERT(usbd_rndis_ops->usb_rndis_cep_rx != 0);

    if(0 != usbd_rndis_ops->usb_rndis_cep_rx)
        usbd_rndis_ops->usb_rndis_cep_rx(status);
    return USB_SUCCESS;
}

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif
