/*******************************************************************************
 * (c) Copyright 2012-2015 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack.
 *      USB Logical Layer (USB-LL)
 *          USBD-Printer class driver.
 *
 *  USBD-Printer class driver public API.
 *
 * SVN $Revision: 7461 $
 * SVN $Date: 2015-06-11 13:48:10 +0530 (Thu, 11 Jun 2015) $
 */
/*=========================================================================*//**
  @mainpage Smartfusion2 MSS USB driver
                USB Logical Layer (USB-LL)
                    USBD-Printer driver
  @section intro_sec Introduction
  The Printer Class device driver implements the USB Printer device as specified
  by the USB-IF. This driver enables easy implementation of printer functionality
  on the Smarfusion2 devices.

  This driver implements the Printer class device using high speed Bi-directional
  Interface. The bi-directional interface supports sending data to the printer
  via a Bulk OUT endpoint, and receiving status and other information from the
  printer via a Bulk IN endpoint. This driver is independent of the data format
  (PDL,PCP) used for sending and receiving data to/from the printer device. This
  printer class driver does not support multi-function printing device. Also,
  this driver supports one USB interface (Bi-directional). Implementing alternate
  interfaces (unidirectional and bi-directional) at the same time is not supported.

  This driver uses the USBD-Class driver template to implement the USB Printer device.

  @section theory_op Theory of Operation
  The following steps are involved in the operation of the USBD-Printer driver:
        •    Configuration
        •    Initialization
        •    Enumeration
        •    Class Specific requests
        •    data transfer

  Configuration
  To use USB driver, the USB driver must first be configured in USB device mode
  using the MSS_USB_PERIPHERAL_MODE constant and to use USBD-Printer class driver
  the constant MSS_USB_DEVICE_PRINTER must be defined.

  Initialization
  The Printer class driver must be initialized using the MSS_USBD_printer_init()
  function. Once initialized, this driver gets configured by the USBD driver
  during the enumeration process.

  The Call-back function MSS_USBD_printer_init() is implemented by this driver
  which will be called by the USBD driver when the host configures the this
  device.

  The USBD_printer_get_descriptor() call back function is passed to the
  application to inform about configuration descriptor request. The application
  should provide the class specific descriptor table to the USBD driver.
  This application should defines descriptor table which contains a configuration
  descriptor, an interface descriptor, two endpoint descriptors for successful
  enumeration as a Printer class device.

  Class Specific requests
  The USBD_printer_process_request() callback function is called on receiving a
  class specific request from the USB Host. The class specific request is used to
  read the device ID string, printer current status and to flush all buffer and
  reset the Bulk IN and Bulk OUt to default states. Device ID includes PDLs such
  as PostScript and manufacture ID string which are specific to the printer.
  This driver passes this request to the application as a call-back function.
  The application must process the class specific requests

  Data transfer
  The Printer class driver performs the data transfers using one BULK IN endpoint
  and one BULK OUT endpoint. This driver implements the usbd_printer_tx_complete_cb()
  and the usbd_printer_rx_cb() call back function to get the information on data
  transfer events on the USB bus which are called by the USBD Driver.

  To initiate the transfer this driver provides MSS_USBD_PRINTER_write() and
  MSS_USBD_PRINTER_read_prepare() function to send data to the USB host on the
  Bulk IN endpoint and to receive data from the USB host on the Bulk OUT endpoint
  respectively. After completion of the data transfer which was initiated by the
  MSS_USBD_PRINTER_write() function, the usb_printer_tx_complete call-back function
  will be called to indicate the status of the transfer. After completion of the
  data transfer which was initiated by the MSS_USBD_PRINTER_read_prepare() function,
  the usb_printer_rx call-back function will be called to indicate the status of
  the transfer.

*/
#ifndef __MSS_USB_DEVICE_PRINTER_H_
#define __MSS_USB_DEVICE_PRINTER_H_

#include <stdint.h>
#include "mss_usb_std_def.h"
#include "mss_usb_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED
/*******************************************************************************
 * User Descriptor lengths
 */

/* Full configuration descriptor length*/
#define FULL_CONFIG_DESCR_LENGTH            (USB_STD_CONFIG_DESCR_LEN + \
                                             USB_STD_INTERFACE_DESCR_LEN + \
                                             USB_STD_ENDPOINT_DESCR_LEN + \
                                             USB_STD_ENDPOINT_DESCR_LEN )

#define PRINTER_CLASS_BULK_RX_EP             MSS_USB_RX_EP_1
#define PRINTER_CLASS_BULK_TX_EP             MSS_USB_TX_EP_1
#define PRINTER_CLASS_INTERFACE_NUM          0x00

/***************************************************************************//**
 Exported Types from USBD-Printer class driver
 */
/***************************************************************************//**
  mss_usbd_printer_state_t
  The mss_usbd_printer_state_t provides a type to identify the current state of
  the Printer class driver.
    USBD_PRINTER_NOT_CONFIGURED – The USB Printer class driver is not configured
                                  and it cannot perform the data transfers.
    USBD_PRINTER_CONFIGURED     - The USB Printer class driver is configured by
                                  the host and it can perform the data transfers.
*/
typedef enum mss_usbd_printer_state {
    USBD_PRINTER_NOT_CONFIGURED,
    USBD_PRINTER_CONFIGURED,
}mss_usbd_printer_state_t;

/***************************************************************************//**
  mss_usbd_printer_cb
  The mss_usbd_printer_cb provides the prototype for all the call-back
  functions which must be implemented by the user application. The user
  application must define and initialize a structure of this type and provide
  the address of that structure as parameter to the
  MSS_USBD_PRINTER_set_cb_handler() function.

  usb_printer_init
  The function pointed by the usb_printer_init function pointer will be called
  on receiving a SET_CONFIGURATION request from the USB Host. This function
  configures transmit and receive endpoint as per parameter provided with this
  function and inform to the application about Printer initialization event.

  usb_printer_release
  The function pointed to by the usb_printer_release function pointer is called on
  receiving a CLEAR_CONFIGURATION request from the host with a cfgidx = 0. This
  value is passed as a parameter. This call-back function will also be called
  when the device disconnect event is detected. The cfgidx value will be set to
  0xFF in this case. The application must implement this call-back.

  usb_printer_get_descriptor
  The function pointed by usb_printer_get_descriptor function is called on
  receiving a GET_DESCRIPTOR requesting configuration descriptor. The application
  must implement this call-back.

  usb_printer_process_request
  The function pointed by usb_printer_process_request function is called on
  receiving a class specific request from the USB Host. The application must
  implement this call-back.

  usb_printer_tx_complete
  The function pointed by the usb_printer_tx_complete function pointer is called by
  on completion of the Current Data Transmissions (IN Transaction) which was
  previously initiated using MSS_USBD_tx_ep_configure(). The application
  must implement this call-back.

  usb_printer_rx
  The function pointed by the usb_printer_rx function pointer is called on
  completion of data reception (OUT transaction) which was previously initiated
  using MSS_USBD_PRINTER_read_prepare(). The application must implement this
  call-back.

  usb_printer_cep_tx_complete
  The function pointed by the usb_printer_cep_tx_complete function pointer is called
  when data transmission initiated by the USBD-Printer driver on Control Endpoint
  is complete. The application must implement this call-back if required by the
  specific PDL PCP. The parameter status indicates error status of the transmit
  transaction. A non-zero status value indicates that there was error in last
  transmit transaction.

  usb_printer_cep_rx
  The function pointed by the usb_printer_cep_rx function pointer is called
  when data reception initiated by the USBD-Printer driver on the control endpoint
  is complete. The application must implement this call-back if required by the
  specific PDL PCP. The parameter status indicates error status of the transmit
  transaction. A non-zero status value indicates that there was error in last
  transmit transaction.
 */
typedef struct mss_usbd_printer_cb  {
    uint8_t(*usb_printer_init)(uint8_t cfgidx, mss_usb_device_speed_t musb_speed);
    uint8_t(*usb_printer_release)(uint8_t cfgidx);

    uint8_t*(*usb_printer_get_descriptor)(uint8_t recepient,
                                        uint8_t type,
                                        uint32_t* length,
                                        mss_usb_device_speed_t musb_speed);

    uint8_t (*usb_printer_process_request)(mss_usbd_setup_pkt_t* setup_pkt,
                                         uint8_t** buf_p,
                                         uint32_t* length);

    uint8_t(*usb_printer_tx_complete)(mss_usb_ep_num_t num, uint8_t status);
    uint8_t(*usb_printer_rx)(mss_usb_ep_num_t num,
                             uint8_t status,
                             uint32_t rx_count);

    uint8_t(*usb_printer_cep_tx_complete)(uint8_t status);
    uint8_t(*usb_printer_cep_rx)(uint8_t status);

} mss_usbd_printer_cb_t ;

extern uint8_t conf_descr[FULL_CONFIG_DESCR_LENGTH];
extern uint8_t fs_conf_descr[FULL_CONFIG_DESCR_LENGTH];

/***************************************************************************//**
  The MSS_USBD_printer_init() function must be used to initializes the
  USBD-Printer driver.

  @param
    This function does not take any parameters

  @return
    This function does not return any value.

  Example:
  @code
    // Initialize Printer class driver.
    MSS_USBD_printer_init();

    @endcode
 */
void
MSS_USBD_printer_init
(
    void
);

/***************************************************************************//**
  The MSS_USBD_PRINTER_write() function must be used to sends data on the Bulk
  IN endpoint to the USB host.

  @param buf
    The buf parameter is a pointer to the buffer whether the port status and
    additional information will be written by application.

  @param len
    The len parameter specifies the total length port status and additional
    information buffer in bytes.

  @return
    This function does not return a value.

  Example:
  @code
    // Initialize Printer class driver.
    MSS_USBD_printer_init();
    MSS_USBD_PRINTER_set_cb_handler(&mss_usbd_printer);
    MSS_USBD_PRINTER_write( buf, len);

  @endcode

 */
void
MSS_USBD_PRINTER_write
(
    uint8_t* buf,
    uint32_t len
);

/***************************************************************************//**
  The MSS_USBD_PRINTER_set_cb_handler() function must be used to provides the
  call-back functions to the USBD-Printer driver which are implemented by the
  application.

  @param user_desc_cb
    The user_desc_cb parameter provides the address of the structure of type
    mss_usbd_user_descriptors_cb_t which is implemented by the class driver or
    the application.

  @return
    This function does not return a value.

  Example:
  @code
    // Initialize Printer class driver.
    MSS_USBD_printer_init();
    MSS_USBD_PRINTER_set_cb_handler(&mss_usbd_printer);
    MSS_USBD_PRINTER_write( buf, len);

  @endcode
 */
void
MSS_USBD_PRINTER_set_cb_handler
(
    mss_usbd_printer_cb_t* user_desc_cb
);

/***************************************************************************//**
  The MSS_USBD_PRINTER_read_prepare() function must be used to prepares the Bulk
  OUT endpoint to receive data from the USB host.

  @param buf
    The buf parameter is a pointer to the buffer whether the received bytes
    will be stored.

  @param len
    The len parameter specifies the total length received data in bytes.

  @return
    This function does not return a value.

  Example
  @code
    // Initialize Printer class driver.
    MSS_USBD_printer_init();
    MSS_USBD_PRINTER_set_cb_handler(&mss_usbd_printer);
    MSS_USBD_PRINTER_read_prepare(buffer,100);
  @endcode

 */
void
MSS_USBD_PRINTER_read_prepare
(
    uint8_t* buf,
    uint32_t len
);

/***************************************************************************//**
  The MSS_USBD_printer_get_state() function can be used to find out the current
  state of the USB-Printer driver.

  @param
    This function does not take a parameter.

  @return
    This function returns a value of type mss_usbd_printer_state_t indicating
    the current state of the Printer class driver.

  Example
  @code
    MSS_USBD_printer_get_state()
  @endcode

 */
mss_usbd_printer_state_t
MSS_USBD_printer_get_state
(
    void
);

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_PRINTER_H_ */
