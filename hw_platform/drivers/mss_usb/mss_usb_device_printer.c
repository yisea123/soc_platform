/*******************************************************************************
 * (c) Copyright 2012-2015 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBD-Printer class driver.
 *
 * USBD-Printer class driver implementation:
 * This source file implements Printer class functionality.
 *
 * V2.4 Naming convention change, other cosmetic changes.
 * 
 * SVN $Revision: 7515 $
 * SVN $Date: 2015-07-02 14:47:49 +0530 (Thu, 02 Jul 2015) $
 */
#include "mss_usb_device.h"
#include "../../CMSIS/mss_assert.h"
#include "mss_usb_device_printer.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED

/*Values should be same as described in ep descriptors*/
#define PRINTER_TX_EP_MAX_PKT_SIZE_HS                       512
#define PRINTER_RX_EP_MAX_PKT_SIZE_HS                       512

/***************************************************************************//**
  Types Internal to the USBD-Printer class driver
 */
typedef enum mss_usbd_printer_events {
    PRINTER_EVENT_IDLE,
    PRINTER_EVENT_TX,
    PRINTER_EVENT_RX,
    PRINTER_EVENT_TX_ERROR,
    PRINTER_EVENT_RX_ERROR
} mss_usbd_printer_events_t;

/***************************************************************************//**
 Local functions used by USBD-Printer class driver.
 */
/***************************************************************************//**
 Implementations of Call-back functions used by USBD.
 */
static uint8_t usbd_printer_init_cb(uint8_t cfgidx, mss_usb_device_speed_t musb_speed);
static uint8_t usbd_printer_release_cb(uint8_t cfgidx);
static uint8_t usbd_printer_tx_complete_cb(mss_usb_ep_num_t num, uint8_t status);
static uint8_t usbd_printer_cep_tx_complete_cb(uint8_t status);
static uint8_t usbd_printer_cep_rx_cb(uint8_t status);

static uint8_t* usbd_printer_get_descriptor_cb
(
    uint8_t recepient,
    uint8_t type,
    uint32_t* length,
    mss_usb_device_speed_t musb_speed
);

static uint8_t usbd_printer_rx_cb
(
    mss_usb_ep_num_t num,
    uint8_t status,
    uint32_t rx_count
);

static uint8_t usbd_printer_process_request_cb
(
    mss_usbd_setup_pkt_t* setup_pkt,
    uint8_t** buf_pp,
    uint32_t* length
);

/*******************************************************************************
 Global variables used by USBD-Printer class driver.
 */
/* This variable tracks the current state of the USBD-Printer driver. */
mss_usbd_printer_state_t g_usbd_printer_state = USBD_PRINTER_NOT_CONFIGURED;

/* The g_printer_events is used to know the current event in the BOT transfer */
volatile mss_usbd_printer_events_t g_printer_events = PRINTER_EVENT_IDLE;

/* Definition for printer application call-backs. */
mss_usbd_printer_cb_t *g_mss_usbd_printer_ops;

/* Printer class call-back function. */
mss_usbd_class_cb_t usb_printer_class_cb =
{
    usbd_printer_init_cb,
    usbd_printer_release_cb,
    usbd_printer_get_descriptor_cb,
    usbd_printer_process_request_cb,
    usbd_printer_tx_complete_cb,
    usbd_printer_rx_cb,
    usbd_printer_cep_tx_complete_cb,
    usbd_printer_cep_rx_cb
};

uint8_t conf_descr[FULL_CONFIG_DESCR_LENGTH] =
{
    /*----------------------- Configuration Descriptor -----------------------*/
    USB_STD_CONFIG_DESCR_LEN,                       /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,              /* bDescriptorType */
    FULL_CONFIG_DESCR_LENGTH,                       /* wTotalLength LSB */
    0x00u,                                          /* wTotalLength MSB */
    0x01u,                                          /* bNumInterfaces */
    0x01u,                                          /* bConfigurationValue */
    0x04u,                                          /* iConfiguration */
    0xC0u,                                          /* bmAttributes */
    0x32u,                                          /* bMaxPower */
    /*------------------------- Interface Descriptor -------------------------*/
    USB_STD_INTERFACE_DESCR_LEN,                    /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,                  /* bDescriptorType */
    PRINTER_CLASS_INTERFACE_NUM,                    /* bInterfaceNumber */
    0x00u,                                          /* bAlternateSetting */
    0x02u,                                          /* bNumEndpoints */
    0x07u,                                          /* bInterfaceClass */
    0x01u,                                          /* bInterfaceSubClass */
    0x02u,                                          /* bInterfaceProtocol */
    0x05u,                                          /* bInterface */
    /*------------------------- IN Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    0x81u,                                          /* bEndpointAddress */
    0x02u,                                          /* bmAttributes */
    0x00u,                                          /* wMaxPacketSize LSB */ //22
    0x02u,                                          /* wMaxPacketSize MSB */ //23
    0xFFu,                                          /* bInterval *///ignored by host for Bulk IN EP
    /*------------------------- OUT Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    0x01u,                                          /* bEndpointAddress */
    0x02u,                                          /* bmAttributes */
    0x00u,                                          /* wMaxPacketSize LSB *///29
    0x02u,                                          /* wMaxPacketSize MSB *///30
    0xFFu                                           /* bInterval *//*Max NAK rate*/
};

uint8_t fs_conf_descr[FULL_CONFIG_DESCR_LENGTH] =
{
    /*----------------------- Configuration Descriptor -----------------------*/
    USB_STD_CONFIG_DESCR_LEN,                       /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,              /* bDescriptorType */
    FULL_CONFIG_DESCR_LENGTH,                       /* wTotalLength LSB */
    0x00u,                                          /* wTotalLength MSB */
    0x01u,                                          /* bNumInterfaces */
    0x01u,                                          /* bConfigurationValue */
    0x04u,                                          /* iConfiguration */
    0xC0u,                                          /* bmAttributes */
    0x32u,                                          /* bMaxPower */
    /*------------------------- Interface Descriptor -------------------------*/
    USB_STD_INTERFACE_DESCR_LEN,                    /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,                  /* bDescriptorType */
    PRINTER_CLASS_INTERFACE_NUM,                    /* bInterfaceNumber */
    0x00u,                                          /* bAlternateSetting */
    0x02u,                                          /* bNumEndpoints */
    0x07u,                                          /* bInterfaceClass */
    0x01u,                                          /* bInterfaceSubClass */
    0x02u,                                          /* bInterfaceProtocol */
    0x05u,                                          /* bInterface */
    /*------------------------- IN Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    (0x80u | PRINTER_CLASS_BULK_TX_EP),             /* bEndpointAddress */
    0x02u,                                          /* bmAttributes */
    0x00u,                                          /* wMaxPacketSize LSB */ //22
    0x02u,                                          /* wMaxPacketSize MSB */ //23
    0xFFu,                                          /* bInterval *///ignored by host for Bulk IN EP
    /*------------------------- OUT Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    PRINTER_CLASS_BULK_RX_EP,                       /* bEndpointAddress */
    0x02u,                                          /* bmAttributes */
    0x00u,                                          /* wMaxPacketSize LSB *///29
    0x02u,                                          /* wMaxPacketSize MSB *///30
    0xFFu                                           /* bInterval *//*Max NAK rate*/
};

/***************************************************************************//**
  See mss_usb_device_printer.h for details.
 */
void
MSS_USBD_printer_init
(
    void
)
{
    MSS_USBD_set_class_cb_handler(&usb_printer_class_cb);
}

/***************************************************************************//**
  See mss_usb_device_printer.h for details.
 */
void
MSS_USBD_PRINTER_set_cb_handler
(
    mss_usbd_printer_cb_t* user_desc_cb
)
{
    g_mss_usbd_printer_ops = user_desc_cb;
}

/***************************************************************************//**
  See mss_usb_device_printer.h for details.
 */
void
MSS_USBD_PRINTER_write
(
    uint8_t* buf,
    uint32_t len
)
{
    MSS_USBD_tx_ep_write(PRINTER_CLASS_BULK_TX_EP, buf, len);
}

/***************************************************************************//**
  See mss_usb_device_printer.h for details.
 */
void
MSS_USBD_PRINTER_read_prepare
(
    uint8_t* buf,
    uint32_t len
)
{
    MSS_USBD_rx_ep_read_prepare(PRINTER_CLASS_BULK_RX_EP, (uint8_t*)buf, len);
}

/***************************************************************************//**
  Function to read the printer driver state.
 */
mss_usbd_printer_state_t
MSS_USBD_printer_get_state
(
    void
)
{
    return g_usbd_printer_state;
}

/***************************************************************************//**
  Call back function definition.
 */
/***************************************************************************//**
 usbd_printer_init_cb() call-back is called by USB Device mode driver on
 receiving SET_CONFIGURATION command. This function configure the transmit and
 receive endpoint as per parameter provided with this function and inform to the
 application about Printer initialization event.
 */
static uint8_t
usbd_printer_init_cb
(
    uint8_t cfgidx,
    mss_usb_device_speed_t musb_speed
)
{
    uint16_t bulk_rxep_fifo_sz = 0u;
    uint16_t bulk_rxep_maxpktsz = 0u;
    uint16_t bulk_txep_fifo_sz = 0u;
    uint16_t bulk_txep_maxpktsz = 0u;

    g_printer_events = PRINTER_EVENT_IDLE;
    g_usbd_printer_state = USBD_PRINTER_NOT_CONFIGURED;

    bulk_txep_fifo_sz = (uint16_t)((conf_descr[23u] << 8u) | (conf_descr[22u]));
    bulk_txep_maxpktsz = (uint16_t)((conf_descr[23u] << 8u) | (conf_descr[22u]));
    bulk_rxep_fifo_sz = (uint16_t)((conf_descr[30u] << 8u) | (conf_descr[29u]));
    bulk_rxep_maxpktsz = (uint16_t)((conf_descr[30u] << 8u) | (conf_descr[29u]));

    /*NO_ZLP_TO_XFR is selected for both endpoints because the Printers
    generally have a transfer level protocol (Similar to SCSI in MSC class)
    by which the transfer size is known before starting the transfer.
    So, a ZLP is not required to terminate the BULK transfer when the transfer
    size is WMaxPktlen or its multiple*/
    MSS_USBD_rx_ep_configure(PRINTER_CLASS_BULK_RX_EP,
                             0x100u,
                             bulk_rxep_fifo_sz,
                             bulk_rxep_maxpktsz,
                             1u,
                             DMA_ENABLE,
                             MSS_USB_DMA_CHANNEL1,
                             MSS_USB_XFR_BULK,
                             NO_ZLP_TO_XFR);

    MSS_USBD_tx_ep_configure(PRINTER_CLASS_BULK_TX_EP,
                             0x200u,
                             bulk_txep_fifo_sz,
                             bulk_txep_maxpktsz,
                             1u,
                             DMA_ENABLE,
                             MSS_USB_DMA_CHANNEL2,
                             MSS_USB_XFR_BULK,
                             NO_ZLP_TO_XFR);

    /* Inform to the application. */
    if(g_mss_usbd_printer_ops->usb_printer_init != 0)
    {
        g_mss_usbd_printer_ops->usb_printer_init(cfgidx, musb_speed);
    }

    g_usbd_printer_state = USBD_PRINTER_CONFIGURED;

    return USB_SUCCESS;
}

/***************************************************************************//**
  usbd_printer_release_cb() call-back is called on receiving a CLEAR_CONFIGURATION
  request from the host. This function calls application call-back function
  pointed by usb_printer_release function to clear the configuration.
 */
static uint8_t
usbd_printer_release_cb
(
    uint8_t cfgidx
)
{
    /* Inform to the application about clear configuration command. */
    if(g_mss_usbd_printer_ops->usb_printer_release != 0)
    {
        g_mss_usbd_printer_ops->usb_printer_release(cfgidx);
    }

    g_usbd_printer_state = USBD_PRINTER_NOT_CONFIGURED;

    return USB_SUCCESS;
}

/***************************************************************************//**
  usbd_printer_get_descriptor_cb() function is called on receiving a GET_DESCRIPTOR
  request from the USB Host. This function internally calls application call-back
  function pointed by usb_printer_get_descriptor() function to returns the
  configuration descriptor requested by Host.
*/
static uint8_t*
usbd_printer_get_descriptor_cb
(
    uint8_t recepient,
    uint8_t type,
    uint32_t* length,
    mss_usb_device_speed_t musb_speed
)
{
    uint8_t* conf_descripter = 0x00u;

    /* Inform to the application about GET_DESCRIPTOR request from USB Host. */
    if(g_mss_usbd_printer_ops->usb_printer_get_descriptor != 0)
    {
        conf_descripter = g_mss_usbd_printer_ops->usb_printer_get_descriptor(recepient,
                                                                            type,
                                                                            length,
                                                                            musb_speed);
    }

    return conf_descripter;
}

/***************************************************************************//**
  usbd_printer_process_request_cb() function is called on receiving a class
  specific request from the USB Host. This function internally calls the
  application call-back function pointed by usb_printer_process_request function
  to process the class specific request.
 */
static uint8_t
usbd_printer_process_request_cb
(
    mss_usbd_setup_pkt_t* setup_pkt,
    uint8_t** buf_pp,
    uint32_t* length
)
{
    uint8_t status = 0x0u;

    if(g_mss_usbd_printer_ops->usb_printer_process_request != 0)
    {
        status = g_mss_usbd_printer_ops->usb_printer_process_request(setup_pkt,
                                                                    buf_pp,
                                                                    length);
    }
    return status;
}

/***************************************************************************//**
  usbd_printer_tx_complete_cb() call-back function is called on completion of the
  Current Data Transmissions (IN Transaction) which was previously initiated
  using MSS_USBD_tx_ep_configure(). This function calls the application
  call-back function pointed by usb_printer_tx_complete function to inform about
  completion of IN transaction.
 */
static uint8_t usbd_printer_tx_complete_cb
(
    mss_usb_ep_num_t num,
    uint8_t status
)
{
    if(status & (TX_EP_UNDER_RUN_ERROR | TX_EP_STALL_ERROR) )
    {
        g_printer_events = PRINTER_EVENT_TX_ERROR;
    }
    else
    {
        g_printer_events = PRINTER_EVENT_TX;
    }

    /* Inform to the application about IN transaction completion. */
    if(g_mss_usbd_printer_ops->usb_printer_tx_complete != 0)
    {
        g_mss_usbd_printer_ops->usb_printer_tx_complete(num, status);
    }

    return USB_SUCCESS;
}

/***************************************************************************//**
  usbd_printer_rx_cb() call-back function is called on completion of data
  reception. This function calls the application call-back function pointed by
  usb_printer_rx function to inform about completion of OUT transaction.
 */
static uint8_t
usbd_printer_rx_cb
(
    mss_usb_ep_num_t num,
    uint8_t status,
    uint32_t rx_count
)
{
    if(status & (RX_EP_OVER_RUN_ERROR | RX_EP_STALL_ERROR |
                 RX_EP_DATA_ERROR | RX_EP_PID_ERROR | RX_EP_ISO_INCOMP_ERROR))
    {
        g_printer_events = PRINTER_EVENT_RX_ERROR;
    }
    else
    {
        g_printer_events = PRINTER_EVENT_RX;
    }

    /* Inform to the application about OUT transaction completion. */
    if(g_mss_usbd_printer_ops->usb_printer_rx != 0)
    {
        g_mss_usbd_printer_ops->usb_printer_rx(num, status, rx_count);
    }

    return USB_SUCCESS;
}

/***************************************************************************//**
  usbd_printer_cep_tx_complete_cb() function is called when data transmission
  initiated by the USBD-Printer driver on Control Endpoint is complete.
*/
static uint8_t
usbd_printer_cep_tx_complete_cb
(
    uint8_t status
)
{
    uint8_t return_status = 0x00u;

    if(g_mss_usbd_printer_ops->usb_printer_cep_tx_complete != 0)
    {
        return_status = g_mss_usbd_printer_ops->usb_printer_cep_tx_complete(status);
    }
    return return_status;
}

/***************************************************************************//**
  usbd_printer_cep_rx_cb() function is called when data reception initiated by
  the USBD-Printer driver on the control endpoint is complete.
*/
static uint8_t
usbd_printer_cep_rx_cb
(
    uint8_t status
)
{
    uint8_t return_status = 0x00u;

    if(g_mss_usbd_printer_ops->usb_printer_cep_rx != 0)
    {
        return_status = g_mss_usbd_printer_ops->usb_printer_cep_rx(status);
    }

    return return_status;
}

#endif //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif
