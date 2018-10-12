#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "usbdrv.h"

#include "mss_assert.h"
#include "mss_usb_device_vendor.h"


#define USBD_RX_TIMEOUT      (10*1000/portTICK_RATE_MS)
#define USBD_TX_TIMEOUT      (20*1000/portTICK_RATE_MS)


extern mss_usbd_user_descr_cb_t vendor_dev_descriptors_cb;

int usb_online;
mss_usb_ep_num_t vendor_tx_ep, vendor_rx_ep;

xSemaphoreHandle sem_usb_rxdata = NULL, sem_usb_txdone = NULL;

extern volatile int tx_completed, rx_completed;
extern volatile uint32_t rxdata_count;
extern void usbd_rx_prepare(uint8_t* buf, uint32_t len);


int usbd_write(const uint8_t* buf, uint32_t len)
{
	/*Make sure that address is Modulo-4.Bits D0-D1 are read only.*/
	ASSERT(!(((uint32_t)buf) & 0x00000002));

	MSS_USBD_tx_ep_write(vendor_tx_ep, (uint8_t *)buf, len);
	if (xSemaphoreTake(sem_usb_txdone, USBD_TX_TIMEOUT) == pdFALSE)
		return -1;	// return -1 when timeout
	return len;
}


int usbd_read(uint8_t* buf, uint32_t len)
{
	/*Make sure that address is Modulo-4.Bits D0-D1 are read only.*/
	ASSERT(!(((uint32_t)buf) & 0x00000002));

	usbd_rx_prepare(buf, len);
	if (xSemaphoreTake(sem_usb_rxdata, USBD_RX_TIMEOUT) == pdFALSE)
		return -1;	// return -1 when timeout

	return rxdata_count;
}


int usbd_install(const struct usbd_config *config)
{
	usb_online = 0;

	vendor_rx_ep = MSS_USB_RX_EP_1;
	vendor_tx_ep = MSS_USB_TX_EP_2;
	tx_completed = 0;
	rx_completed = 0;
	rxdata_count = 0;

	vSemaphoreCreateBinary(sem_usb_rxdata);
	vSemaphoreCreateBinary(sem_usb_txdone);
	if (sem_usb_rxdata == NULL || sem_usb_txdone == NULL)
		return -1;

	/* Initialize USB driver. */
	MSS_USBD_init(MSS_USB_DEVICE_HS);

	/*Assign call-back function Interface needed by USBD driver*/
	MSS_USBD_set_descr_cb_handler(&vendor_dev_descriptors_cb);

	/*Initialize the USBD_VENDOR class driver*/
	MSS_USBD_VENDOR_init(MSS_USB_DEVICE_HS);

	return 0;
}