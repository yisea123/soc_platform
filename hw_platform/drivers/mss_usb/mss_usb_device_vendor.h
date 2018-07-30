/*******************************************************************************
 * (c) Copyright 2012-2015 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack.
 *      USB Logical Layer (USB-LL)
 *          USBD-VENDOR class driver.
 *
 *  USBD-MSC class driver public API.
 *
 * SVN $Revision: 7461 $
 * SVN $Date: 2015-06-11 13:48:10 +0530 (Thu, 11 Jun 2015) $
 */

/*=========================================================================*//**
  @mainpage Smartfusion2 MSS USB driver
                USB Logical Layer (USB-LL)
                    USBD-VENDOR driver

  @section intro_sec Introduction
  The Vendor Class device driver implements an example of the USB vendor class
  device. The Vendor class driver specification offers complete freedom to the
  user to implement the USB device  in a desired way not confirming to any of
  the defined USB classes. With the Vendor class driver, it is possible to define
  the USB device with any combinations of bulk, interrupt and isochronous
  endpoints in addition to the control endpoint. This driver implements one of
  such possible combinations. User may change the device descriptors and the
  driver implementation to suit to their needs. This driver is an example which
  demonstrates the way to implement the Vendor class driver using the underlying
  USB driver stack.

  This driver implements the vendor class USB device with One BULK IN, one
  BULK OUT, one INTERRUPT IN and one INTERRUPT OUT endpoint in addition to the
  control endpoint. This makes it easily possible to use readily available
  WinUSBTestapp application provided by Windows to test the data transfers using
  the Windows host.

  This driver uses the USBD-Class driver template to implement the USB Vendor
  class device.

  @section theory_op Theory of Operation
  The following steps are involved in the operation of the USBD-VENDOR driver:
                •   Configuration
                •   Initialization
                •   Enumeration
                •   Class specific requests
                •   Data transfer

  Configuration
  To use this driver, the MSS USB driver must first be configured in the USB
  device mode using the MSS_USB_PERIPHERAL_MODE macro. No other configuration is
  necessary.

  Initialization
  The Vendor class driver must be initialized using the MSS_USBD_VENDOR_init()
  function. Once initialized, this driver gets configured by the USBD driver
  during the enumeration process. The Call-back function usbd_vendor_init_cb()
  is implemented by this driver which will be called by the USBD driver when the
  USB host configures the this device. All the endpoints defined in the descriptor
  table of this driver get configured for data transfer in this function. If there
  is a USB OUT endpoint defined in the descriptor tables then it will also be
  prepared to receive data using MSS_USBD_rx_ep_read_prepare() function.

  This driver defines descriptor table which contains a configuration descriptor,
  an interface descriptor, and four endpoint descriptors for successful enumeration
  as a vendor class device. The call-back function usbd_vendor_get_descriptor_cb()
  is implemented to provide the class specific descriptor table to the USBD driver.

  Note: For successful enumeration, the device specific descriptors must also be
  provided by the application using the MSS_USBD_set_desc_cb_handler()
  function to the USBD Driver. Since the device descriptor, string descriptors
  etc. are not class specific, they are not part of the Vendor class driver.

  Class Specific requests
  The usbd_vendor_process_request_cb() call-back function is implemented by
  this driver which processes the MSC class specific requests. Vendor class
  driver can implement vendor specific requests (D6..5 of bmRequestType field
  of the USB request has value 2) which may or may not have a data stage to it.
  Moreover, the data stage can have data moving either from the USB device to
  the USB host or vice versa. All types of vendor specific requests are supported.
  If there is a data stage in the vendor specific request and the data direction
  is from USB device to the USB host (i.e. IN transaction) then a buffer and its
  length will be provided to the USBD driver in the
  usbd_vendor_process_request_cb() function. When the data stage is complete,
  the usbd_vendor_cep_tx_complete_cb() will be called to mark the completion of the
  data stage, the error status will also be provided as parameter. If there is a
  data stage in the vendor specific request and the data direction is from USB
  host to the USB device (i.e. OUT transaction) then a buffer and its length will
  be provided to the USBD driver, where the received data shall be kept, in the
  usbd_vendor_process_request_cb() function. When the data stage is complete,
  the usbd_vendor_cep_rx_cb() function will be called to mark the completion
  of the data stage, the error status will also be provided as parameter. When
  there is no data stage, then the request must be processed in the
  usbd_vendor_process_request_cb() function itself.

  Data transfer
  This USB Vendor class device driver performs data transfers over a pair of
  Interrupt endpoint (IN and OUT) and a pair of bulk endpoints (IN and OUT). This
  driver implements the usbd_vendor_tx_complete_cb() and the usbd_vendor_rx_cb()
  functions to get the information on data transfer events on the USB bus which
  are called by the USBD Driver. The endpoint number and the error status of the
  data transfer on the corresponding endpoint are communicated as parameters of
  these call-back functions. The usbd_vendor_tx_complete_cb() call-back function
  is called when the previously initiated IN transfer is complete. The vendor
  class driver implementation must take further action depending on the endpoint
  number and the error status. Similarly, the call-back function
  usbd_vendor_rx_cb() is called when the previously initiated OUT
  transfer is complete. The vendor class driver implementation must take further
  action depending on the endpoint number and the error status.

  This vendor class driver implementation performs the data transfers with USB
  host as described below. The WinUSBTestapp is expected to be running on the
  Windows host. Since user can implement data transfer as per need, this data
  transfer implementation is serving as an example and may not actually accomplish
  any defined purpose.

  The interrupt OUT and the Bulk OUT endpoints are prepared to read the OUT
  transfers at the device configuration time. On receiving the data from the USB
  host, fixed amount of data is transferred from USB device to the USB host
  (IN transaction) on the corresponding IN endpoint. The OUT endpoint is again
  made ready to receive maximum packet data in the next OUT transaction. E.g.
  If data is received on the Interrupt OUT endpoint then a fixed amount of data
  is transmitted to the host on the Interrupt IN endpoint.

  This way all the possible data transfer types can be tested with WinUSBTestapp.
  Please note that for every IN data transfer on Interrupt and Bulk IN endpoint
  you must already know the amount of data bytes to be received from the vendor
  class device. This driver transfers 64 bytes with every OUT transfer over
  Interrupt as well as Bulk endpoint.

  Note that with the Bulk transfer there are two possibilities to indicate the
  end of the data transfer. In the first case, the data to be transferred is
  already known (as in BoT protocol for MSC devices). In this case, a short
  packet is not required to indicate the end of Bulk transfer when the total data
  transfer is exact multiple of WMaxPacket size. In second case, the data transfer
  size is not known beforehand; hence a short packet is always used to indicate
  the end of the transfer. In this case when the data transfer length is multiple
  of WMaxPacket length, a ZLP (zero length packet) short packet must be transferred
  to indicate the end of the transferred. To take care of these conditions, the
  endpoint must be configured appropriately using the constant ADD_ZLP_TO_XFR.
  Please see USBD driver details to know more about endpoint configuration functions.
  This driver configures BULK endpoints such that a short packet or ZLP is
  required to indicate the end of the bulk transfer.

*//*==========================================================================*/

#ifndef __MSS_USB_DEVICE_VENDOR_H_
#define __MSS_USB_DEVICE_VENDOR_H_

#include "mss_usb_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED

/*******************************************************************************
  Exported functions from USBD driver
 *******************************************************************************/

/***************************************************************************//**
  @brief MSS_USBD_VENDOR_init()
  The MSS_USBD_VENDOR_init() function must be used by the application to initialize
  the VENDOR class driver.

  @param speed
  The speed parameter indicates the USB speed at which this class driver must
  operate.

  @return
    This function does not return a value.

  Example:
  @code
        //Initialize the USBD-Vendor class driver
        MSS_USBD_VENDOR_init(MSS_USB_DEVICE_HS);

        //Assign call-back function handler structure needed by USB Device Core driver
        MSS_USBD_set_desc_cb_handler(&vendor_dev_descr_cb);

        //Initialize USB driver HS device mode
        MSS_USBD_init(MSS_USB_DEVICE_HS);
  @endcode
*/
void
MSS_USBD_VENDOR_init
(
    mss_usb_device_speed_t speed
);


#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_VENDOR_H_ */
