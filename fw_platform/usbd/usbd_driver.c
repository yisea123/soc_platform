#include <stdio.h>

#ifndef USB_BAREMETAL
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#include "usbdrv.h"

#include "hal.h"
#include "mss_assert.h"
#include "mss_usb_device_vendor.h"


#define USBD_RX_TIMEOUT      (2*1000/portTICK_RATE_MS)
#define USBD_TX_TIMEOUT      (2*1000/portTICK_RATE_MS)


extern mss_usbd_user_descr_cb_t vendor_dev_descriptors_cb;

int usb_online;
mss_usb_ep_num_t vendor_tx_ep, vendor_rx_ep;

#ifndef USB_BAREMETAL
xSemaphoreHandle sem_usb_rxdata = NULL, sem_usb_txdone = NULL;
#define USBD_ENTER_CRITICAL()	taskENTER_CRITICAL()
#define USBD_EXIT_CRITICAL()	taskEXIT_CRITICAL()
#else
psr_t saved_psr;
#define USBD_ENTER_CRITICAL()	(saved_psr = HAL_disable_interrupts())
#define USBD_EXIT_CRITICAL()	HAL_restore_interrupts(saved_psr)
#endif

static uint8_t rxbuffer[2560];
static uint32_t rxbuffer_len = sizeof(rxbuffer);

volatile int rx_put_index, rx_get_index;

volatile uint32_t rxdata_count;

extern volatile int tx_completed, rx_completed;
extern void usbd_rx_prepare(void);

void usbd_rxdata_reset()
{
	rxdata_count = 0;
	rx_get_index = rx_put_index = 0;
}

	

uint32_t usbd_get_buffer_data_count(void)
{
	uint32_t get_tmp, put_tmp;

	get_tmp = rx_get_index;
	put_tmp = rx_put_index;
	return ((put_tmp >= get_tmp)?(put_tmp - get_tmp):(rxbuffer_len - get_tmp + put_tmp));
}

uint32_t usbd_buffer_is_full(void)
{
	uint32_t get_tmp, put_tmp;

	get_tmp = rx_get_index;
	put_tmp = rx_put_index+1;

	if(put_tmp==rxbuffer_len)
		put_tmp = 0;
	if(put_tmp==get_tmp)
		return 1;
	return 0;
}
#define usbd_buffer_is_empty()		(rx_put_index==rx_get_index)

uint32_t usbd_receive_data(uint8_t *buffer, uint32_t len)
{
	uint8_t *src, *dst;
	uint32_t i, cnt;

	if(usbd_buffer_is_full())
		return 0;
	
	src = buffer;
	dst = &rxbuffer[rx_put_index];
	cnt = rxbuffer_len - usbd_get_buffer_data_count()-1;
	len = (cnt>len)?len:cnt;

	for (i = 0; i < len; i++)
	{
		*dst++ = *src++;
		if (++rx_put_index == rxbuffer_len) {
			rx_put_index = 0;
			dst = rxbuffer;
		}
	}
	
	//printf("\n\rusb receive: put=%d, get=%d\n\r", rx_put_index,  rx_get_index);
	return len;
}

int usbd_write(const uint8_t* buf, uint32_t len)
{
	/*Make sure that address is Modulo-4.Bits D0-D1 are read only.*/
	ASSERT(!(((uint32_t)buf) & 0x00000002));
	tx_completed = 0;
	MSS_USBD_tx_ep_write(vendor_tx_ep, (uint8_t *)buf, len);
#ifndef USB_BAREMETAL
	if (xSemaphoreTake(sem_usb_txdone, USBD_TX_TIMEOUT) == pdFALSE)
		return -1;	// return -1 when timeout
#else
	while (!tx_completed) ;
#endif
	return len;
}

int usbd_read(uint8_t* buf, uint32_t len)
{
	uint8_t *src, *dst;
	uint32_t i, cnt, datalen;

	while (usbd_buffer_is_empty())
	{
#ifndef USB_BAREMETAL
		if (xSemaphoreTake(sem_usb_rxdata, USBD_RX_TIMEOUT) == pdFALSE)
			return -1;	// return -1 when timeout
#endif
	}
	dst = buf;
	src = &rxbuffer[rx_get_index];
	datalen = usbd_get_buffer_data_count();
	
	cnt = (datalen < len) ? datalen : len;
	for (i = 0; i < cnt; i++)
	{
		*dst++ = *src++;
		if (++rx_get_index == rxbuffer_len) {
			rx_get_index = 0;
			src = rxbuffer;
		}
	}

	//printf("\n\rusbd_read: put=%d, get=%d\n\r",rx_put_index, rx_get_index);
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

#ifndef USB_BAREMETAL
	vSemaphoreCreateBinary(sem_usb_rxdata);
	vSemaphoreCreateBinary(sem_usb_txdone);
	if (sem_usb_rxdata == NULL || sem_usb_txdone == NULL)
		return -1;
#endif

	/* Initialize USB driver. */
	MSS_USBD_init(MSS_USB_DEVICE_HS);

	/*Assign call-back function Interface needed by USBD driver*/
	MSS_USBD_set_descr_cb_handler(&vendor_dev_descriptors_cb);

	/*Initialize the USBD_VENDOR class driver*/
	MSS_USBD_VENDOR_init(MSS_USB_DEVICE_HS);

	return 0;
}