#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "usbdrv.h"

#include "mss_assert.h"
#include "mss_usb_device_vendor.h"


#define USBD_RX_TIMEOUT      (60*1000/portTICK_RATE_MS)
#define USBD_TX_TIMEOUT      (10*1000/portTICK_RATE_MS)


extern mss_usbd_user_descr_cb_t vendor_dev_descriptors_cb;

int usb_online;
mss_usb_ep_num_t vendor_tx_ep, vendor_rx_ep;

xSemaphoreHandle sem_usb_rxdata = NULL, sem_usb_txdone = NULL;

static uint8_t rxbuffer[2048];
int rx_put_index, rx_get_index;

volatile uint32_t rxdata_count;

extern volatile int tx_completed, rx_completed;
extern void usbd_rx_prepare(uint8_t* buf, uint32_t len);

void usbd_rxdata_reset()
{
	rxdata_count = 0;
	rx_get_index = rx_put_index = 0;
}


uint32_t usbd_receive_data(uint8_t *buffer, uint32_t len)
{
	uint8_t *src, *dst;
	uint32_t i, cnt;

	src = buffer;
	dst = &rxbuffer[rx_put_index];
	cnt = rxdata_count;
	for (i = 0; i < len; i++)
	{
		*dst++ = *src++;
		if (++rx_put_index == sizeof(rxbuffer)) {
			rx_put_index = 0;
			dst = rxbuffer;
		}
		if (++rxdata_count == sizeof(rxbuffer))
			break;
	}
	return rxdata_count - cnt;
}


int usbd_write(const uint8_t* buf, uint32_t len)
{
	/*Make sure that address is Modulo-4.Bits D0-D1 are read only.*/
	ASSERT(!(((uint32_t)buf) & 0x00000002));
	tx_completed = 0;
	MSS_USBD_tx_ep_write(vendor_tx_ep, (uint8_t *)buf, len);
	if (xSemaphoreTake(sem_usb_txdone, USBD_TX_TIMEOUT) == pdFALSE)
		return -1;	// return -1 when timeout
	return len;
}


int usbd_read(uint8_t* buf, uint32_t len)
{
	uint8_t *src, *dst;
	uint32_t i, cnt;

	while (rxdata_count == 0)
	{
		if (xSemaphoreTake(sem_usb_rxdata, USBD_RX_TIMEOUT) == pdFALSE && rxdata_count == 0)
			return -1;	// return -1 when timeout
	}
	dst = buf;
	src = &rxbuffer[rx_get_index];
	taskENTER_CRITICAL();
	cnt = (rxdata_count < len) ? rxdata_count : len;
	for (i = 0; i < cnt; i++)
	{
		*dst++ = *src++;
		if (++rx_get_index == sizeof(rxbuffer)) {
			rx_get_index = 0;
			src = rxbuffer;
		}
	}
	rxdata_count -= cnt;
	taskEXIT_CRITICAL();

	return cnt;
}


int usbd_install(const struct usbd_config *config)
{
	usb_online = 0;

	vendor_rx_ep = MSS_USB_RX_EP_1;
	vendor_tx_ep = MSS_USB_TX_EP_2;
	tx_completed = 0;
	rx_completed = 0;

	usbd_rxdata_reset();

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