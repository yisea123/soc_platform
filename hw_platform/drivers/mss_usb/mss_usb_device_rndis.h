/*******************************************************************************
 * (c) Copyright 2012-2016 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack.
 *      USB Logical Layer (USB-LL)
 *          USBD-RNDIS class driver.
 *
 * USBD-RNDIS class driver public API
 *
 * SVN $Revision: 8276 $
 * SVN $Date: 2016-03-02 09:40:23 +0530 (Wed, 02 Mar 2016) $
 */

/*=========================================================================*//**
  @mainpage Smartfusion2 MSS USB driver.
       USB Logical Layer (USB-LL)
           USBD-RNDIS class driver.

  @section intro_sec Introduction
  This Communications Device Class driver implements the USB CDC class specified
  by the USB-IF for the RNDIS vendor specific protocol. This driver enables
  RNDIS (Remote Network Driver Interface Specification) based communications
  over the USB interface for Smarfusion2 devices.
  With this driver, the Smartfusion2 device appears as an Ethernet port, when
  connected to a USB host. Unlike other USB classes, the USB RNDIS devices need
  multiple USB interfaces to achieve the complete device functionality. All
  these interfaces must be associated with a single device driver on the USB
  host side. This driver uses interface association descriptor (IAD) along with
  other descriptors for enumeration.

  This driver implements two USB interfaces: a communications class interface
  and a data class interface to implement RNDIS. The communications class
  interface is a communications management interface and is required for all
  communications devices. The data class interface is used to transport the
  encapsulated network packet data between the host and the device.

  The communications class interface uses two USB endpoints: a control endpoint
  for enumeration and communication management and an interrupt IN endpoint for
  providing CDC class notifications to the USB host.

  The data class interface uses one BULK IN endpoint and one BULK OUT endpoint
  for data transfers between the host and the device.

  This driver uses the USBD-Class driver template to implement the CDC class
  RNDIS functionality.

  The actual RNDIS protocol which implements network device control and packet
  encapsulation is implemented at the application layer as far as this device
  driver is concerned.

  @section theory_op Theory of Operation
  The following steps are involved in the operations of the USBD-RNDIS class
  driver:
    •    Configuration
    •    Initialization
    •    Enumeration
    •    Class Specific requests
    •    data transfer

  Configuration
  The MSS USB driver must first be configured in the USB device mode using the
  MSS_USB_PERIPHERAL_MODE to use this driver. No other configuration is
  necessary.

  Initialization
  The RNDIS CDC class driver must be initialized using the MSS_USBD_RNDIS_init()
  function. Once initialized, this driver is configured by the USBD device
  driver during the enumeration process. The call-back function
  MSS_USBD_RNDIS_Init_cb() is implemented by this driver which will be called by
  the USBD driver when the host configures this device. The
  MSS_USBD_RNDIS_get_descriptor() function is implemented to provide the class
  specific descriptor table to the USBD driver.

  For successful enumeration as an RNDIS CDC class device, this driver defines a
  descriptor table which contains a configuration descriptor, an interface
  association descriptor, two interface descriptors, CDC class descriptors and
  three endpoint descriptors.

  Note:    The device specific descriptors must be provided for successful
  enumeration by the application using the MSS_USBD_set_desc_cb_handler()
  function to the USBD Driver. Since the device descriptor, string descriptors
  etc. are not class specific, they are not part of the RNDIS CDC Class driver.

  Class Specific requests
  The MSS_USBD_RNDIS_process_request_cb() call-back function is implemented by
  this driver which processes the RNDIS CDC class specific requests. The class
  driver must implement this call-back function to successfully respond to the
  host requests.

  Data transfer
  The RNDIS CDC class driver performs the data transfers on a data class
  interface using one BULK IN endpoint and one BULK OUT endpoint. The data
  transferred on these endpoints are Ethernet packets encapsulated in the
  Microsoft vendor specific RNDIS protocol.
  The application uses the MSS_USBD_RNDIS_tx() and MSS_USBD_RNDIS_rx_prepare()
  functions respectively to transmit and receive data on the data interface.
  This driver implements MSS_USBD_RNDIS_datain_cb() and
  MSS_USBD_RNDIS_dataout_cb() functions which are called by the USBD driver to
  get the information on the data transfer events on the USB bus.

  Note: The RNDIS functionality provides the emulation of Ethernet data transfer
  on the USB bus. Though the received data is presented to the application
  software as Ethernet dta packets, the underlying USB protocol must be
  followed. In the USB protocol, a USB Device cannot transfer data unless the
  USB host requests it. Though the MSS_USBD_CDC_tx() function keeps the data
  ready in the USB TX FIFO, the actual data transmission  takes place only when
  an IN packet is received from the host. This behaviour is different than the
  normal Ethernet transmission where both the connected devices can send
  messages on their own (when no hardware flow control is used).

 *//*=========================================================================*/

#ifndef __MSS_USB_DEVICE_RNDIS_H_
#define __MSS_USB_DEVICE_RNDIS_H_

#include <stdint.h>
#include "mss_usb_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED
/*******************************************************************************
 * Public Types from USBD-RNDIS class driver
 */
/*
  mss_usbd_rndis_state_t
  The mss_usbd_rndis_state_t type is used to provide information of current
  state of the RNDIS class driver.

  USBD_RNDIS_NOT_CONFIGURED --  The USB RNDIS class driver is not configured and
                                can not perform the data transfers.
  USBD_RNDIS_CONFIGURED     --  The USB RNDIS class driver is configured by the
                                host and it can perform the data transfers.
  USBD_RNDIS_TRANSMITTING   --  The USB RNDIS class driver is configured by the
                                host and is transmitting data.
  USBD_RNDIS_RECEIVING      --  The USB RNDIS class driver is configured by the
                                host and is receiving data.
*/

typedef enum mss_usbd_rndis_state {
    USBD_RNDIS_NOT_CONFIGURED,
    USBD_RNDIS_CONFIGURED,
    USBD_RNDIS_TRANSMITTING,
    USBD_RNDIS_RECEIVING
}mss_usbd_rndis_state_t;

/*******************************************************************************
 * Public Constants from USBD-RNDIS class driver
 */
/*******************************************************************************
  RNDIS Class configuration definitions:
  These values will be used by RNDIS driver to configure the MSS USB core
  endpoints. These values are also used for generating the descriptor tables
  which are sent to the USB host during enumeration.

  Note: You are allowed to modify these values according to requirement. Make
  sure that the values defined here are in accordance with the user descriptors
  (device descriptor etc), USB 2.0 standard and the USBD driver operation speed
  (MSS_USB_DEVICE_HS or MSS_USB_DEVICE_FS)
*/

/* MSS USB core Endpoints used by RNDIS class */
#define RNDIS_BULK_OUT_EP                               MSS_USB_RX_EP_1
#define RNDIS_BULK_IN_EP                                MSS_USB_TX_EP_1
#define RNDIS_INTR_IN_EP                                MSS_USB_TX_EP_2

/* MSS USB core FIFO address for endpoints used by RNDIS class */
#define RNDIS_BULK_IN_EP_FIFO_ADDR                      0x100u
#define RNDIS_BULK_OUT_EP_FIFO_ADDR                     0x200u
#define RNDIS_INTR_IN_EP_FIFO_ADDR                      0x50u

/* MSS USB core FIFO size for endpoints used by RNDIS class */
#define RNDIS_BULK_IN_EP_FIFO_SIZE                      64u
#define RNDIS_BULK_OUT_EP_FIFO_SIZE                     64u
#define RNDIS_INTR_IN_EP_FIFO_SIZE                      64u

/* Maximum packet size for endpoints used by RNDIS class */
#define RNDIS_BULK_IN_EP_MAX_PKT_SIZE                   64u
#define RNDIS_BULK_OUT_EP_MAX_PKT_SIZE                  64u
#define RNDIS_INTR_IN_EP_MAX_PKT_SIZE                   64u

/*******************************************************************************
 * Public data strucures from RNDIS class driver.
 ******************************************************************************/

/*******************************************************************************
 mss_usbd_rndis_app_cb_t
  The mss_usbd_rndis_app_cb_t type provides the prototype for all call-back
  function handlers which must be implemented by the the user application.
  The user application must define and initialize a structure of this type and
  provide the address of that structure as parameter when calling the
  MSS_USBD_RNDIS_init() function.

  usb_rndis_init
  The function pointed by the usbd_rndis_init element is called to indicate that
  the RNDIS driver is configured by the USB host. The application can use this
  function to prepare itself for RNDIS protocol control and data exchange with
  the RNDIS miniport driver on the USB host. This driver cannot perform data
  transfers unless it is configured by the USB host to do so.

  usb_rndis_release
  The function pointed by the usbd_rndis_release element is called when the USB
  host wants to un-configure this RNDIS device.

  usb_rndis_process_request
  The function pointed by the usbd_rndis_process_request element is called when
  this driver receives a RNDIS class specific request to be processed. The
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

  usb_rndis_tx_complete
  The function pointed by the usbd_rndis_tx_complete element is called to
  indicate completion of a data transmission operation. The status parameter
  provides the error status of this operation. A non-zero value indicates that
  there was an error in the last operation.

  usb_rndis_rx
  The function pointed by the usbd_rndis_rx element is called to indicate that
  data has been received from the host. The status parameter provides
  the error status of this operation. A non-zero value indicates that there was
  an error in the last operation. The number of bytes received during this
  operation is indicated in the rx_count parameter.

  usb_rndis_notification
  This driver needs to transmit RNDIS class notification data on the interrupt
  endpoint on a periodic basis. The period of transmission of the notification
  data is selected in endpoint descriptor. The host gets this information during
  the enumeration process. The application must provide the notification data to
  this driver by implementing the usb_rndis_notification element of the
  usbd_rndis_app_cb_t type structure. This callback function is called
  periodically when IN token is received on the RNDIS interrupt endpoint.

  usb_rndis_cep_tx_complete
  The function pointed by the usb_rndis_cep_tx_complete element is called when
  the data packet is transmitted on a previously configured control endpoint.
  The data from the buffer provided with the usb_rndis_process_requests
  call-back was transmitted. The parameter status indicates the error status of
  the transmit transaction. A non-zero status value indicates that there was an
  error in the last transmit transaction.

  usb_rndis_cep_rx
  The function pointed by the usb_rndis_cep_rx element is called when a data
  packet is received on a previously configured control endpoint. The data will
  be placed in the buffer which was previously provided with the
  usb_rndis_process_requests call-back. The parameter status indicates the error
  status of the receive transaction. A non-zero status value indicates that
  there were errors in the last receive transaction.
*/
typedef struct mss_usbd_rndis_app_cb {
    void (*usb_rndis_init)(void);
    void (*usb_rndis_release)(void);
    uint8_t (*usb_rndis_process_request)(mss_usbd_setup_pkt_t* setup_pkt,
                                       uint8_t** buf,
                                       uint32_t* length);

    uint8_t(*usb_rndis_tx_complete)(uint8_t status);
    uint8_t(*usb_rndis_rx)(uint8_t status, uint32_t rx_count);
    void(*usb_rndis_notification)(uint8_t** buf_p, uint32_t* length_p);
    uint8_t(*usb_rndis_cep_tx_complete)(uint8_t status);
    uint8_t(*usb_rndis_cep_rx)(uint8_t status);

} mss_usbd_rndis_app_cb_t;

/*******************************************************************************
 * Exported APIs from USBD-RNDIS driver
 ******************************************************************************/

/***************************************************************************//**
  @brief MSS_USBD_RNDIS_init()
  The MSS_USBD_RNDIS_init() function must be used by the application to
  initialize the RNDIS class driver. A pointer to a structure of type
  mss_usbd_rndis_app_cb_t must be passed as a parameter to this function.

  @param rndis_app_cb
  The rndis_app_cb parameter is a pointer to the structure of type
  mss_usbd_rndis_app_cb_t. This will be used by the USBD-RNDIS class driver to
  call the call-back functions implemented by the application.

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
        //Assign call-back function handler structure needed by USBD-RNDIS driver
        MSS_USBD_RNDIS_init(&g_rndis_app_cb, MSS_USB_DEVICE_HS);

        //Assign call-back function handler structure needed by USB Device Core driver
        MSS_USBD_set_desc_cb_handler(&rndis_descr_cb);

        //Initialize USB Device Core driver
        MSS_USBD_init(MSS_USB_DEVICE_HS);

  @endcode
*/
void
MSS_USBD_RNDIS_init
(
    mss_usbd_rndis_app_cb_t* rndis_app_cb,
    mss_usb_device_speed_t speed
);

/***************************************************************************//**
  @brief MSS_USBD_RNDIS_tx()
  The MSS_USBD_RNDIS_tx() function can be used by the application to start
  transmission on the RNDIS data port. A pointer to the buffer which needs to be
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
        if(USBD_RNDIS_CONFIGURED == MSS_USBD_RNDIS_get_state())
        {
        MSS_USBD_RNDIS_tx(data_buf, length);
        }
  @endcode
*/
void
MSS_USBD_RNDIS_tx
(
    uint8_t* buf,
    uint32_t length
);

/***************************************************************************//**
  @brief MSS_USBD_RNDIS_rx_prepare()
  The MSS_USBD_RNDIS_rx_prepare() function can be used by the application to
  prepare the RNDIS device to receive data. Once prepared, this driver will copy
  the data into the provided buffer when it receives the data from the host.

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
        if(USBD_RNDIS_CONFIGURED == MSS_USBD_RNDIS_get_state())
        {
            MSS_USBD_RNDIS_rx_prepare(buf, length);
        }

  @endcode
*/
void
MSS_USBD_RNDIS_rx_prepare
(
    uint8_t* buf,
    uint32_t length
);

/***************************************************************************//**
  @brief MSS_USBD_RNDIS_get_state()
  The MSS_USBD_RNDIS_get_state() function can be used by the application to find
  out the current state of the RNDIS Class driver.

  @param
  This function does not take any parameter.

  @return
  This function returns the status of the RNDIS Class driver of type
  mss_usbd_rndis_state_t.

  Example:
  @code
        if(USBD_RNDIS_CONFIGURED == MSS_USBD_RNDIS_get_state())
        {
            MSS_USBD_RNDIS_rx_prepare(buf, length);
        }
  @endcode
*/
mss_usbd_rndis_state_t
MSS_USBD_RNDIS_get_state
(
    void
);

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_RNDIS_H_ */
