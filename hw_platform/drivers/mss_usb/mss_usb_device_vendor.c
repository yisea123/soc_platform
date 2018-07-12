/*******************************************************************************
 * (c) Copyright 2012-2015 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBD-VENDOR class driver Template.
 *
 * USBD-VENDOR class driver Template:
 * This source file implements a template for Vendor class implementation.
 * This template must be modified to implement the actual vendor specific
 * class functionality.
 *
 * V2.4 Naming convention change, other cosmetic changes.
 * 
 * SVN $Revision: 7515 $
 * SVN $Date: 2015-07-02 14:47:49 +0530 (Thu, 02 Jul 2015) $
 */
#include "mss_usb_device_vendor.h"
#include "mss_usb_device.h"
#include "mss_usb_std_def.h"
#include "../../CMSIS/mss_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED

#define VENDOR_INTR_TX_EP                                 MSS_USB_TX_EP_1
#define VENDOR_INTR_RX_EP                                 MSS_USB_RX_EP_1
#define VENDOR_BULK_TX_EP                                 MSS_USB_TX_EP_2
#define VENDOR_BULK_RX_EP                                 MSS_USB_RX_EP_2

#define VENDOR_CLASS_INTERFACE_NUM                        0x00
/*HS Operation values*/
#define VENDOR_TX_EP_FIFO_SIZE_HS                         512
#define VENDOR_RX_EP_FIFO_SIZE_HS                         512

/*Values should be same as described in ep descriptors*/
#define VENDOR_TX_EP_MAX_PKT_SIZE_HS                      512
#define VENDOR_RX_EP_MAX_PKT_SIZE_HS                      512

/*LS Operation values*/
#define VENDOR_TX_EP_FIFO_SIZE_FS                         64
#define VENDOR_RX_EP_FIFO_SIZE_FS                         64

/*Values should be same as described in ep descriptors*/
#define VENDOR_TX_EP_MAX_PKT_SIZE_FS                      64
#define VENDOR_RX_EP_MAX_PKT_SIZE_FS                      64

/* Full configuration descriptor length*/
#define FULL_CONFIG_DESCR_LENGTH                    (USB_STD_CONFIG_DESCR_LEN + \
                                                     USB_STD_INTERFACE_DESCR_LEN + \
                                                     USB_STD_ENDPOINT_DESCR_LEN + \
                                                     USB_STD_ENDPOINT_DESCR_LEN + \
                                                     USB_STD_ENDPOINT_DESCR_LEN + \
                                                     USB_STD_ENDPOINT_DESCR_LEN )

#define VENDOR_CONF_DESCR_DESCTYPE_IDX                    1u

#define VENDOR_INTR_RX_EP_FIFO_ADDR                       0x100u
#define VENDOR_INTR_RX_EP_FIFO_SIZE                       64u
#define VENDOR_INTR_RX_EP_MAX_PKT_SIZE                    64u

#define VENDOR_INTR_TX_EP_FIFO_ADDR                       0x140u
#define VENDOR_INTR_TX_EP_FIFO_SIZE                       64u
#define VENDOR_INTR_TX_EP_MAX_PKT_SIZE                    64u

#define VENDOR_BULK_RX_EP_FIFO_ADDR                       0x180u

#define VENDOR_BULK_TX_EP_FIFO_ADDR                       0x380u

/***************************************************************************//**
 Implementations of Call-back functions used by USBD.
 */
static uint8_t* usbd_vendor_get_descriptor_cb(uint8_t recepient,
                                              uint8_t type,
                                              uint32_t* length,
                                              mss_usb_device_speed_t musb_speed);

static uint8_t usbd_vendor_init_cb(uint8_t cfgidx, mss_usb_device_speed_t musb_speed);
static uint8_t usbd_vendor_tx_complete_cb(mss_usb_ep_num_t num, uint8_t status);
static uint8_t usbd_vendor_rx_cb(mss_usb_ep_num_t num, uint8_t status, uint32_t rx_count);

static uint8_t usbd_vendor_process_request_cb(mss_usbd_setup_pkt_t* setup_pkt,
                                              uint8_t** buf_pp,
                                              uint32_t* length);

static uint8_t usbd_vendor_cep_tx_complete_cb(uint8_t status);
static uint8_t usbd_vendor_cep_rx_cb(uint8_t status);

/*******************************************************************************
 Global variables used by USBD-VENDOR class driver.
 */

/* USB current Speed of operation selected by user*/
mss_usb_device_speed_t g_usbd_vendor_user_speed;

#if defined(__GNUC__)
static uint8_t g_req_tx_data[64] __attribute__ ((aligned (4))) = "Example instruction data";
static uint8_t g_bulk_tx_data[64] __attribute__ ((aligned (4))) = "Bulk Endpoint data";
static uint8_t g_intr_tx_data[64] __attribute__ ((aligned (4))) = "Interrupt Endpoint data";

static uint8_t g_req_rx_data[64] __attribute__ ((aligned (4))) = {0};
static uint8_t g_bulk_rx_data[64] __attribute__ ((aligned (4))) = {0};
static uint8_t g_intr_rx_data[64] __attribute__ ((aligned (4))) = {0};
#elif defined(__ICCARM__)
#pragma data_alignment = 4
static uint8_t g_req_tx_data[64] = "Example instruction data";
static uint8_t g_bulk_tx_data[64]="Bulk Endpoint data";
static uint8_t g_intr_tx_data[64]="Interrupt Endpoint data";

static uint8_t g_req_rx_data[64]={0};
static uint8_t g_bulk_rx_data[64]={0};
static uint8_t g_intr_rx_data[64]={0};
#elif defined(__CC_ARM)
__align(4) static uint8_t g_req_tx_data[64] = "Example instruction data";
__align(4) static uint8_t g_bulk_tx_data[64]="Bulk Endpoint data";
__align(4) static uint8_t g_intr_tx_data[64]="Interrupt Endpoint data";

__align(4) static uint8_t g_req_rx_data[64]={0};
__align(4) static uint8_t g_bulk_rx_data[64]={0};
__align(4) static uint8_t g_intr_rx_data[64]={0};
#endif

mss_usbd_class_cb_t usb_vendor_class_cb = {usbd_vendor_init_cb,
                                           0,
                                           usbd_vendor_get_descriptor_cb,
                                           usbd_vendor_process_request_cb,
                                           usbd_vendor_tx_complete_cb,
                                           usbd_vendor_rx_cb,
                                           usbd_vendor_cep_tx_complete_cb,
                                           usbd_vendor_cep_rx_cb };

uint8_t vendor_fs_conf_descr[FULL_CONFIG_DESCR_LENGTH] =
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
    VENDOR_CLASS_INTERFACE_NUM,                     /* bInterfaceNumber */
    0x00u,                                          /* bAlternateSetting */
    0x04u,                                          /* bNumEndpoints */
    0xFFu,                                          /* bInterfaceClass */
    0xFFu,                                          /* bInterfaceSubClass */
    0xFFu,                                          /* bInterfaceProtocol */
    0x05u,                                          /* bInterface */
    /*------------------------- Interrupt IN Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    (0x80u |VENDOR_INTR_TX_EP),                     /* bEndpointAddress */
    0x03u,                                          /* bmAttributes --Interrupt */
    0x40u,                                          /* wMaxPacketSize LSB */ //22
    0x00u,                                          /* wMaxPacketSize MSB */ //23
    0xFFu,                                          /* bInterval */
    /*------------------------- Interrupt OUT Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    VENDOR_INTR_RX_EP,                             /* bEndpointAddress */
    0x03u,                                          /* bmAttributes -- Interrupt */
    0x40u,                                          /* wMaxPacketSize LSB *///29
    0x00u,                                          /* wMaxPacketSize MSB *///30
    0xFFu,                                          /* bInterval *//*Max NAK rate*/
    /*------------------------- Bulk IN Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    (0x80u | VENDOR_BULK_TX_EP),                    /* bEndpointAddress */
    0x02u,                                          /* bmAttributes --Bulk */
    0x40u,                                          /* wMaxPacketSize LSB */ //22
    0x00u,                                          /* wMaxPacketSize MSB */ //23
    0xFFu,                                          /* bInterval *///ignored by host for Bulk IN EP
    /*--------------------- Bulk  OUT endpoint Descriptor ----------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    VENDOR_BULK_RX_EP,                             /* bEndpointAddress */
    0x02u,                                          /* bmAttributes -- Bulk */
    0x40u,                                          /* wMaxPacketSize LSB *///29
    0x00u,                                          /* wMaxPacketSize MSB *///30
    0xFFu                                           /* bInterval */ /*Max NAK rate*/
};

/*
 Configuration descriptor and sub-ordinate descriptors to enumerate the USB device
 as Mass Storage class Device by host.
 */
uint8_t vendor_hs_conf_descr[FULL_CONFIG_DESCR_LENGTH] =
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
    VENDOR_CLASS_INTERFACE_NUM,                     /* bInterfaceNumber */
    0x00u,                                          /* bAlternateSetting */
    0x04u,                                          /* bNumEndpoints */
    0xFFu,                                          /* bInterfaceClass */
    0xFFu,                                          /* bInterfaceSubClass */
    0xFFu,                                          /* bInterfaceProtocol */
    0x05u,                                          /* bInterface */
    /*------------------------- Interrupt IN Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    (0x80u |VENDOR_INTR_TX_EP),                     /* bEndpointAddress */
    0x03u,                                          /* bmAttributes --Interrupt */
    0x40u,                                          /* wMaxPacketSize LSB */ //22
    0x00u,                                          /* wMaxPacketSize MSB */ //23
    0xFFu,                                          /* bInterval */  //ignored by host for Bulk IN EP
    /*------------------------- Interrupt OUT Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    VENDOR_INTR_RX_EP,                              /* bEndpointAddress */
    0x03u,                                          /* bmAttributes -- Interrupt */
    0x40u,                                          /* wMaxPacketSize LSB *///29
    0x00u,                                          /* wMaxPacketSize MSB *///30
    0xFFu,                                          /* bInterval *//*Max NAK rate*/
    /*------------------------- Bulk IN Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    (0x80u | VENDOR_BULK_TX_EP),                    /* bEndpointAddress */
    0x02u,                                          /* bmAttributes --Bulk */
    0x00u,                                          /* wMaxPacketSize LSB */ //22
    0x02u,                                          /* wMaxPacketSize MSB */ //23
    0xFFu,                                          /* bInterval *///ignored by host for Bulk IN EP
    /*------------------------- Bulk  Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                     /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    VENDOR_BULK_RX_EP,                             /* bEndpointAddress */
    0x02u,                                          /* bmAttributes -- Bulk */
    0x00u,                                          /* wMaxPacketSize LSB *///29
    0x02u,                                          /* wMaxPacketSize MSB *///30
    0xFFu                                           /* bInterval */ /*Max NAK rate*/
};

void
MSS_USBD_VENDOR_init
(
    mss_usb_device_speed_t speed
)
{
    g_usbd_vendor_user_speed = speed;

    MSS_USBD_set_class_cb_handler(&usb_vendor_class_cb);
}

static uint8_t
usbd_vendor_init_cb
(
    uint8_t cfgidx,
    mss_usb_device_speed_t musb_speed
)
{
    uint16_t bulk_rxep_fifo_sz = 0u;
    uint16_t bulk_rxep_maxpktsz = 0u;

    uint16_t bulk_txep_fifo_sz = 0u;
    uint16_t bulk_txep_maxpktsz = 0u;

    /*
      User Selected FS: Operate only in FS
      User Selected HS:
        Device connected to 2.0 Host(musb_speed = HS):Operate in HS
        Device connected to 1.x Host(musb_speed = FS):Operate in FS
      */

      /*
      We calculate the Bulk endpoint sizes and max_pkt_size based on the
      respective endpoint speed. This is done so that the config descriptor change
      reflects here and this function need not be updated for that.
      */
    if(MSS_USB_DEVICE_FS == musb_speed)
    {
        bulk_txep_fifo_sz = (uint16_t)((vendor_fs_conf_descr[23u] << 8u) | (vendor_fs_conf_descr[22u]));
        bulk_txep_maxpktsz = (uint16_t)((vendor_fs_conf_descr[23u] << 8u) | (vendor_fs_conf_descr[22u]));
        bulk_rxep_fifo_sz = (uint16_t)((vendor_fs_conf_descr[30u] << 8u) | (vendor_fs_conf_descr[29u]));
        bulk_rxep_maxpktsz = (uint16_t)((vendor_fs_conf_descr[30u] << 8u) | (vendor_fs_conf_descr[29u]));
    }
    else if(MSS_USB_DEVICE_HS == musb_speed)
    {
        bulk_txep_fifo_sz = (uint16_t)((vendor_hs_conf_descr[23u] << 8u) | (vendor_hs_conf_descr[22u]));
        bulk_txep_maxpktsz = (uint16_t)((vendor_hs_conf_descr[23u] << 8u) | (vendor_hs_conf_descr[22u]));
        bulk_rxep_fifo_sz = (uint16_t)((vendor_hs_conf_descr[30u] << 8u) | (vendor_hs_conf_descr[29u]));
        bulk_rxep_maxpktsz = (uint16_t)((vendor_hs_conf_descr[30u] << 8u) | (vendor_hs_conf_descr[29u]));
    }
    else
    {
        ASSERT(0);
    }
    MSS_USBD_rx_ep_configure(VENDOR_INTR_RX_EP,
                             VENDOR_INTR_RX_EP_FIFO_ADDR,
                             VENDOR_INTR_RX_EP_FIFO_SIZE,
                             VENDOR_INTR_RX_EP_MAX_PKT_SIZE,
                             1u,
                             DMA_ENABLE,
                             MSS_USB_DMA_CHANNEL1,
                             MSS_USB_XFR_INTERRUPT,
                             NO_ZLP_TO_XFR);

    MSS_USBD_rx_ep_read_prepare(VENDOR_INTR_RX_EP,
                                (uint8_t*)&g_intr_rx_data,
                                sizeof(g_intr_rx_data));

    MSS_USBD_tx_ep_configure(VENDOR_INTR_TX_EP,
                             VENDOR_INTR_TX_EP_FIFO_ADDR,
                             VENDOR_INTR_TX_EP_FIFO_SIZE,
                             VENDOR_INTR_TX_EP_MAX_PKT_SIZE,
                             1u,
                             DMA_ENABLE,
                             MSS_USB_DMA_CHANNEL2,
                             MSS_USB_XFR_INTERRUPT,
                             NO_ZLP_TO_XFR);

    MSS_USBD_rx_ep_configure(VENDOR_BULK_RX_EP,
                             VENDOR_BULK_RX_EP_FIFO_ADDR,
                             bulk_rxep_fifo_sz,
                             bulk_rxep_maxpktsz,
                             1u,
                             DMA_ENABLE,
                             MSS_USB_DMA_CHANNEL3,
                             MSS_USB_XFR_BULK,
                             NO_ZLP_TO_XFR);

    MSS_USBD_rx_ep_read_prepare(VENDOR_BULK_RX_EP,
                                (uint8_t*)&g_bulk_rx_data,
                                sizeof(g_bulk_rx_data));

    MSS_USBD_tx_ep_configure(VENDOR_BULK_TX_EP,
                             VENDOR_BULK_TX_EP_FIFO_ADDR,
                             bulk_txep_fifo_sz,
                             bulk_txep_maxpktsz,
                             1u,
                             DMA_ENABLE,
                             MSS_USB_DMA_CHANNEL4,
                             MSS_USB_XFR_BULK,
                             NO_ZLP_TO_XFR);

    return USB_SUCCESS;
}

static uint8_t* usbd_vendor_get_descriptor_cb(uint8_t recepient,
                                              uint8_t type,
                                              uint32_t* length,
                                              mss_usb_device_speed_t musb_speed)
{
    uint8_t* conf_desc=0;
    uint8_t* os_conf_desc=0;
    uint8_t conf_desc_len=0;
    uint8_t os_conf_desc_len=0;

    /*User Selected FS:
        Operate only in FS
      User Selected HS:
        Device connected to 2.0 Host(musb_speed = HS):Operate in HS
        Device connected to 1.x Host(musb_speed = FS):Operate in FS
    */
    if(MSS_USB_DEVICE_FS == g_usbd_vendor_user_speed)
    {
        conf_desc = vendor_fs_conf_descr;
        conf_desc[VENDOR_CONF_DESCR_DESCTYPE_IDX] =
                                            USB_CONFIGURATION_DESCRIPTOR_TYPE;

        conf_desc_len = sizeof(vendor_fs_conf_descr);
        os_conf_desc = 0u;
        os_conf_desc_len = 0u;
    }
    else if(MSS_USB_DEVICE_HS == g_usbd_vendor_user_speed)
    {
        if(MSS_USB_DEVICE_HS == musb_speed)
        {
            conf_desc = vendor_hs_conf_descr;
            conf_desc[VENDOR_CONF_DESCR_DESCTYPE_IDX] =
                                              USB_CONFIGURATION_DESCRIPTOR_TYPE;

            conf_desc_len = sizeof(vendor_hs_conf_descr);
            os_conf_desc = vendor_fs_conf_descr;
            os_conf_desc[VENDOR_CONF_DESCR_DESCTYPE_IDX] =
                                        USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE;

            os_conf_desc_len = sizeof(vendor_fs_conf_descr);
        }
        else if(MSS_USB_DEVICE_FS ==musb_speed)
        {
            conf_desc = vendor_fs_conf_descr;
            conf_desc[VENDOR_CONF_DESCR_DESCTYPE_IDX] =
                                             USB_CONFIGURATION_DESCRIPTOR_TYPE;

            conf_desc_len = sizeof(vendor_fs_conf_descr);
            os_conf_desc = vendor_hs_conf_descr;
            os_conf_desc[1u] = USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE;
            os_conf_desc_len = sizeof(vendor_hs_conf_descr);
        }
    }
    else
    {
        ASSERT(0);      //user must select FS or HS, nothing else.
    }

    if(USB_STD_REQ_RECIPIENT_DEVICE == recepient)
    {
        if(USB_CONFIGURATION_DESCRIPTOR_TYPE == type)
        {
           *length = conf_desc_len;
            return(conf_desc);
        }
        else if(USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE == type)
        {
            ASSERT(os_conf_desc != 0u);
            *length = os_conf_desc_len;
            return(os_conf_desc);
        }
    }
    else if(USB_STD_REQ_RECIPIENT_ENDPOINT == recepient)
    {
        /*Do nothing*/
    }
    else if(USB_STD_REQ_RECIPIENT_INTERFACE == recepient)
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
 usbd_vendor_process_request_cb() call-back function Process the VENDOR class
 requests.
 */
static uint8_t
usbd_vendor_process_request_cb
(
    mss_usbd_setup_pkt_t* setup_pkt,
    uint8_t** buf_pp,
    uint32_t* length
)
{
    if(setup_pkt->length)
    {
        if(setup_pkt->request_type & USB_STD_REQ_DATA_DIR_MASK) /*DtoH*/
        {
            *buf_pp = (uint8_t*)&g_req_tx_data;
            *length = setup_pkt->length;
            return USB_SUCCESS;
        }
        else
        {
            /*CEP configuration and read_prepare is taken care by USBD.
              Here we only pass the buffer address
              Once the data is received the callback function
              usbd_vendor_cep_rx_cb will be called by USBD driver.
              The actual processing of the data shall be done then.*/
            *buf_pp = (uint8_t*)&g_req_rx_data;
            *length = setup_pkt->length;
            return USB_SUCCESS;
        }

    }
    else
    {
        return USB_SUCCESS;
    }
}

/***************************************************************************//**
 usbd_vendor_tx_complete_cb() call-back function is called by USB Device mode driver
 on completion of the Current Data Transmissions (IN Transaction) which was
 previously initiated using MSS_USBD_tx_ep_configure().
 */
static uint8_t usbd_vendor_tx_complete_cb
(
    mss_usb_ep_num_t num,
    uint8_t status
)
{
    if(status & (TX_EP_UNDER_RUN_ERROR | TX_EP_STALL_ERROR) )
    {
        /* Take error mitigation action based on the error indication "status" */
    }
    else
    {
        if (VENDOR_INTR_TX_EP == num)
        {
            MSS_USBD_rx_ep_read_prepare(VENDOR_INTR_RX_EP,
                                        (uint8_t*)&g_intr_rx_data,
                                        sizeof(g_intr_rx_data));
        }
        else if (VENDOR_BULK_TX_EP == num)
        {
            MSS_USBD_rx_ep_read_prepare(VENDOR_BULK_RX_EP,
                                        (uint8_t*)&g_bulk_rx_data,
                                        sizeof(g_bulk_rx_data));
        }
        else
        {
            ASSERT(0); /*Endpoint number not as per descriptors.*/
        }
    }

    return USB_SUCCESS;
}

/***************************************************************************//**
 usbd_vendor_rx_cb() call-back function is called by USB Device mode driver
 on completion of data reception. USB Device mode driver must have been
 previously prepared for this data reception using
 MSS_USBD_rx_ep_read_prepare()
 */
static uint8_t
usbd_vendor_rx_cb
(
    mss_usb_ep_num_t num,
    uint8_t status,
    uint32_t rx_count
)
{
    if(status & (RX_EP_OVER_RUN_ERROR | RX_EP_STALL_ERROR |
             RX_EP_DATA_ERROR | RX_EP_PID_ERROR | RX_EP_ISO_INCOMP_ERROR))
    {
        /* Take error mitigation action based on the error indication "status" */
    }
    else
    {
        if (VENDOR_INTR_TX_EP == num)
        {
            MSS_USBD_tx_ep_write(VENDOR_INTR_TX_EP,
                                 g_intr_tx_data,
                                 sizeof(g_intr_tx_data));
        }
        else if (VENDOR_BULK_TX_EP == num)
        {
            MSS_USBD_tx_ep_write(VENDOR_BULK_TX_EP,
                                 g_bulk_tx_data,
                                 sizeof(g_bulk_tx_data));
        }
        else
        {
            ASSERT(0); /*Endpoint number not as per descriptors.*/
        }

    }

    return USB_SUCCESS;
}

static uint8_t usbd_vendor_cep_tx_complete_cb(uint8_t status)
{
    /*
     If there was data phase in the IN control transfer then this callback
     function is called at the end of the IN data phase of that control transfer.
     The status parameter of this function indicates the error status of the OUT
     data phase.
     */
    return USB_SUCCESS;
}

static uint8_t usbd_vendor_cep_rx_cb(uint8_t status)
{
    /*
     If there was data phase in the OUT control transfer then this callback
     function is called at the end of OUT data phase of that control transfer.
     You can process the data now. The status parameter of this function
     indicates the error status of the OUT data phase.
     */

    return USB_SUCCESS;
}
#endif //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif
