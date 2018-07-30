/*******************************************************************************
 * (c) Copyright 2012-2015 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack.
 *      USB Logical Layer (USB-LL)
 *          USBD-CDC class driver.
 *
 * USBD-CDC class driver public API
 *
 * SVN $Revision: 7461 $
 * SVN $Date: 2015-06-11 13:48:10 +0530 (Thu, 11 Jun 2015) $
 */

/*=========================================================================*//**
  @mainpage Smartfusion2 MSS USB driver.
       USB Logical Layer (USB-LL)
           USBD-CDC class driver.

  @section intro_sec Introduction
  The Communications Device Class driver implements the USB CDC class specified
  by the USB-IF. This driver enables Virtual COM Port (VCP) emulation on the
  Smarfusion2 devices. The VCP functionality falls under PSTN devices subclass
  of the USB CDC class. With this driver, the Smartfusion2 device appears as a
  COM port, when connected to a USB host. Unlike other USB classes, the USB CDC
  devices need multiple USB interfaces to achieve the complete device
  functionality. All these interfaces must be associated with a single device
  driver on the USB host side. This driver uses interface association descriptor
  (IAD) along with other descriptors for enumeration.

  This driver implements two USB interfaces, communications class interface and
  data class interface to implement VCP. The communications class interface is
  a communications management interface and is required for all communications
  devices. The data class interface is used to transport the data between the
  host and the device.

  The communications class interface uses two USB endpoints: control endpoint
  for enumeration and communication management and an interrupt IN endpoint for
  providing CDC class notifications to the USB host.

  The data class interface uses one BULK IN endpoint and one BULK OUT endpoint
  for data transfers between the host and the device.

  This driver uses the USBD-Class driver template to implement the CDC class VCP
  functionality.

  @section theory_op Theory of Operation
  The following steps are involved in the operations of the USBD-CDC class driver:
    •    Configuration
    •    Initialization
    •    Enumeration
    •    Class Specific requests
    •    data transfer

  Configuration
  The MSS USB driver must first be configured in the USB device mode using the
  MSS_USB_PERIPHERAL_MODE to use this driver. No other configuration is necessary.

  Initialization
  The CDC class driver must be initialized using the MSS_USBD_CDC_init() function.
  Once initialized, this driver is configured by the USBD device driver during
  the enumeration process. The call-back function MSS_USBD_CDC_Init_cb() is
  implemented by this driver which will be called by the USBD driver when the
  host configures this device. The MSS_USBD_CDC_get_descriptor() function is
  implemented to provide class specific descriptor table to the USBD driver.

  For successful enumeration as a CDC class device, this driver defines
  descriptor table which contains a configuration descriptor, interface
  association descriptor, two interface descriptors, CDC class descriptors and
  three endpoint descriptors.

  Note:    The device specific descriptors must be provided for successful
  enumeration by the application using the MSS_USBD_set_desc_cb_handler()
  function to the USBD Driver. Since the device descriptor, string descriptors
  etc. are not class specific, they are not part of the CDC Class driver.

  Class Specific requests
  The MSS_USBD_CDC_process_request_cb() call-back function is implemented by
  this driver which processes the CDC class specific requests. The class driver
  must implement this call-back function to successfully respond to the host
  requests.

  Data transfer
  The CDC class driver performs the data transfers on data class interface using
  one BULK IN endpoint and one BULK OUT endpoint. The data transferred on these
  endpoints is raw data (no protocol abstraction on top of the USB transfers).
  The application must use the MSS_USBD_CDC_tx()  and the
  MSS_USBD_CDC_rx_prepare() functions to transmit and receive data on VCP
  respectively. This driver implements CDC_tx_complete_cb() and CDC_rx_cb()
  function which are called by USBD driver to get the information on the data
  transfer events on the USB bus.

  Note: The VCP functionality is the emulation of serial data transfer on the
  USB bus. Though the received data is presented to the application software as
  raw serial data, the underlying USB protocol must be followed. In USB protocol,
  a USB Device cannot transfer data unless the USB host requests for it. Though
  the MSS_USBD_CDC_tx() function keeps the data ready in the USB TX FIFO, the
  actual data transmission  takes place only when a IN packet is received from
  the host. This behaviour is different than the UART serial transmission where
  both the connected devices can send messages own their own (When no flow is
  control used).

 *//*=========================================================================*/

#ifndef __MSS_USB_DEVICE_CDC_H_
#define __MSS_USB_DEVICE_CDC_H_

#include <stdint.h>
#include "mss_usb_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED
/*******************************************************************************
 * Public Types from USBD-CDC class driver
 */
/*
  mss_usbd_cdc_state_t
  The mss_usbd_cdc_state_t type is used to provide information of current state
  of the CDC class driver. This type is intended to be used by the application
  to know if this driver is been configured by the USB host. i.e. the enumeration
  is complete and data transfers can be started.

  USBD_CDC_NOT_CONFIGURED   --  The USB CDC class driver is not configured and
                                can not perform the data transfers.
  USBD_CDC_CONFIGURED       --  The USB CDC class driver is configured by the
                                host and it can perform the data transfers.
*/

typedef enum mss_usbd_cdc_state {
    USBD_CDC_NOT_CONFIGURED,
    USBD_CDC_CONFIGURED
}mss_usbd_cdc_state_t;

/*******************************************************************************
 * Public Constants from USBD-CDC class driver
 */
/*******************************************************************************
  CDC Class configuration definitions:
  These values will be used by CDC driver to configure the MSS USB core endpoints.
  These values are also used for generating the descriptor tables which are sent
  to the USB host during enumeration.

  Note: You are allowed to modify these values according to requirement. Make
  sure that the values defined here are in accordance with the user descriptors
  (device descriptor etc.), USB 2.0 standard and the USBD driver operation speed
  (MSS_USB_DEVICE_HS or MSS_USB_DEVICE_FS)
*/

/*MSS USB core Endpoints used by CDC class*/
#define VCP_BULK_RX_EP                                  MSS_USB_RX_EP_1
#define VCP_BULK_TX_EP                                  MSS_USB_TX_EP_1
#define VCP_INTR_TX_EP                                  MSS_USB_TX_EP_2

/*MSS USB core FIFO address for endpoints used by CDC class*/
#define VCP_BULK_TX_EP_FIFO_ADDR                        0x100u
#define VCP_BULK_RX_EP_FIFO_ADDR                        0x400u
#define VCP_INTR_TX_EP_FIFO_ADDR                        0x50u

/*MSS USB core FIFO size for endpoints used by CDC class*/
#define VCP_BULK_TX_EP_FIFO_SIZE                        64u
#define VCP_BULK_RX_EP_FIFO_SIZE                        64u
#define VCP_INTR_TX_EP_FIFO_SIZE                        64u

/*Maximum packet size for endpoints used by CDC class*/
#define VCP_BULK_TX_EP_MAX_PKT_SIZE                     64u
#define VCP_BULK_RX_EP_MAX_PKT_SIZE                     64u
#define VCP_INTR_TX_EP_MAX_PKT_SIZE                     64u

/*******************************************************************************
 * Public data structures from CDC class driver.
 ******************************************************************************/

/*******************************************************************************
 mss_usbd_cdc_app_cb_t
  The mss_usbd_cdc_app_cb_t type provides the prototype for all call-back
  function handlers which must be implemented by the the user application.
  The user application must define and initialize a structure of this type and
  provide the address of that structure as parameter when calling the
  MSS_USBD_CDC_init() function.

  usb_cdc_init
  The function pointed by the usbd_cdc_init element is called to indicate that
  the CDC driver is configured by the USB host. The application can use this
  function to prepare itself for data exchange with the USB host. This driver
  cannot perform data transfers unless it is configured by the USB host to do so.

  usb_cdc_release
  The function pointed by the usbd_cdc_release element is called when the USB
  host un-configures this CDC device. The CDC class device is un-configured when
  it receives SET_CONFIGURATION request from USB host with a cfgidx = 0. This
  value is passed as a parameter. In case when the disconnect event is detected
  by the USBD driver a value of cfgidx = 0xFF is passed. The application can use
  this call-back function and its parameter to take appropriate action as required.

  usb_cdc_process_request
  The function pointed by the usbd_cdc_process_request element is called when
  this driver receives a CDC class specific request to be processed. The
  parameter setup_pkt is the pointer to the setup packet sent by the USB host.
  The parameters buf_p and length are used for the data which might be
  associated with the current setup packet. If the host requests data from the
  device, the application must provide the address of the buffer containing the
  data in buf_p parameter and the length of this buffer in bytes in length
  parameter. If the host wants to send data to the device then application must
  provide the address of the buffer where the data must be placed in buf_p
  parameter and the size of the buffer in bytes in the length parameter. The
  buf_p and the length parameters are not meaningful for the zero data length
  requests.

  usb_cdc_tx_complete
  The function pointed by the usbd_cdc_tx_complete element is called to indicate
  completion of data transmission operation. The status parameter provides the
  error status of this operation. A non-zero value indicates that there was an
  error in the last operation.

  usb_cdc_rx
  The function pointed by the usbd_cdc_rx element is called to indicate that the
  data is available to read from the RX EP FIFO. The status parameter provides
  the error status of this operation. A non-zero value indicates that there was
  an error in the last operation. Number of bytes received during this operation
  is indicated in the rx_count parameter.

  usb_cdc_notification
  This driver needs to transmit CDC class notification data on the interrupt
  endpoint on a periodic basis. The period of transmission of the notification
  data is selected in endpoint descriptor. The host gets this information during
  the enumeration process. The application must provide the notification data to
  this driver by implementing the usb_cdc_notification element of the
  usbd_cdc_app_cb_t type structure. This callback function is called
  periodically when IN token is received on the CDC interrupt endpoint.

  usb_cdc_cep_tx_complete
  The function pointed by the usb_cdc_cep_tx_complete element is called when
  the data packet is transmitted on previously configured control endpoint.
  The data from the buffer provided with the usb_cdc_process_requests call-back
  was transmitted. The parameter status indicates the error status of the
  transmit transaction. A non-zero status value indicates that there was an
  error in the last transmit transaction.

  usb_cdc_cep_rx
  The function pointed by the usb_cdc_cep_rx element is called when a data
  packet is received on previously configured control endpoint. The data will be
  placed in the buffer which was previously provided with the
  usb_cdc_process_requests call-back. The parameter status indicates the error
  status of the receive transaction. A non-zero status value indicates that
  there were errors in the last receive transaction.
*/
typedef struct mss_usbd_cdc_app_cb {
    void (*usb_cdc_init)(void);
    void (*usb_cdc_release)(uint8_t cfgidx);
    uint8_t (*usb_cdc_process_request)(mss_usbd_setup_pkt_t* setup_pkt,
                                       uint8_t** buf,
                                       uint32_t* length);

    uint8_t(*usb_cdc_tx_complete)(uint8_t status);
    uint8_t(*usb_cdc_rx)(uint8_t status, uint32_t rx_count);
    void(*usb_cdc_notification)(uint8_t** buf_p, uint32_t* length_p);
    uint8_t(*usb_cdc_cep_tx_complete)(uint8_t status);
    uint8_t(*usb_cdc_cep_rx)(uint8_t status);

} mss_usbd_cdc_app_cb_t;

/*******************************************************************************
 * Exported APIs from USBD-CDC driver
 ******************************************************************************/

/***************************************************************************//**
  @brief MSS_USBD_CDC_init()
  The MSS_USBD_CDC_init() function must be used by the application to initialize
  the CDC class driver. A pointer to the structure of type mss_usbd_cdc_app_cb_t
  must be passed as a parameter to this function.

  @param vcp_app_cb
  The vcp_app_cb parameter is a pointer to the structure of type
  mss_usbd_cdc_app_cb_t. This will be used by the USBD-CDC class driver to call
  the call-back functions implemented by the application.

  @param speed
  The speed parameter specifies the USB speed at which the USB driver and the
  MSS USB hardware block operate.

  The valid values for this parameter are
        MSS_USB_DEVICE_HS
        MSS_USB_DEVICE_FS

  @return
    This function does not return a value.

  Example:
  @code
        //Assign call-back function handler structure needed by USBD-CDC driver
        MSS_USBD_CDC_init(&g_vcp_app_cb, MSS_USB_DEVICE_HS);

        //Assign call-back function handler structure needed by USB Device Core driver
        MSS_USBD_set_desc_cb_handler(&vcp_descr_cb);

        //Initialize USB Device Core driver
        MSS_USBD_init(MSS_USB_DEVICE_HS);

  @endcode
*/
void
MSS_USBD_CDC_init
(
    mss_usbd_cdc_app_cb_t* vcp_app_cb,
    mss_usb_device_speed_t speed
);

/***************************************************************************//**
  @brief MSS_USBD_CDC_tx()
  The MSS_USBD_CDC_tx() function can be used by the application to start
  transmission on the CDC VCP port. A pointer to the buffer which needs to be
  transmitted must be provided in the parameter buf. The length of the buffer
  must be provided in the parameter length.

  @param buf
  The buf parameter is a pointer to the buffer from which the data is to be
  transmitted.

  @param length
  The length parameter indicates the number of data byte to be transmitted.

  @return
  This function does not return a value.

  Example:
  @code
        if(USBD_CDC_CONFIGURED == MSS_USBD_CDC_get_state())
        {
        MSS_USBD_CDC_tx(data_buf, length);
        }
  @endcode
*/
void
MSS_USBD_CDC_tx
(
    uint8_t* buf,
    uint32_t length
);

/***************************************************************************//**
  @brief MSS_USBD_CDC_rx_prepare()
  The MSS_USBD_CDC_rx_prepare() function can be used by the application to
  prepare the CDC VCP to receive data. Once prepared, this driver will copy the
  data into the provided buffer when it receives the data from the host.

  @param buf
  The buf parameter is a pointer to the buffer in which the received data is to
  be stored.

  @param length
  The length parameter indicates the length of the buffer in which the data
  received from the host is stored.

  @return
  This function does not return a value.

  Example:
  @code
        if(USBD_CDC_CONFIGURED == MSS_USBD_CDC_get_state())
        {
            MSS_USBD_CDC_rx_prepare(buf, length);
        }

  @endcode
*/
void
MSS_USBD_CDC_rx_prepare
(
    uint8_t* buf,
    uint32_t length
);

/***************************************************************************//**
  @brief MSS_USBD_CDC_get_state()
  The MSS_USBD_CDC_get_state() function can be used by the application to find
  out the current state of the CDC Class driver.

  @param
  This function does not take any parameter.

  @return
  This function returns the status of the CDC Class driver of type
  mss_usbd_cdc_state_t.

  Example:
  @code
        if(USBD_CDC_CONFIGURED == MSS_USBD_CDC_get_state())
        {
            MSS_USBD_CDC_rx_prepare(buf, length);
        }
  @endcode
*/
mss_usbd_cdc_state_t
MSS_USBD_CDC_get_state
(
    void
);

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_CDC_H_ */
