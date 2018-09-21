
#include "usbdrv.h"

#include "mss_usb_device_vendor.h"


extern mss_usbd_user_descr_cb_t vendor_dev_descriptors_cb;

mss_usb_ep_num_t vendor_tx_ep, vendor_rx_ep;

extern volatile int tx_completed, rx_completed, rxdata_count, rxdata_index;
extern void usbd_rx_prepare(void);
extern void usbd_rx_datacopy(uint8_t* buf, uint32_t len);

int usbd_write(const uint8_t* buf, uint32_t len)
{
	uint32_t timeout = 0xffffffff;
	tx_completed = 0;
	rxdata_count = 0;

	MSS_USBD_tx_ep_write(vendor_tx_ep, (uint8_t *)buf, len);
	while (!tx_completed && timeout != 0) {
		--timeout;
	}
	return timeout ? len : -1;
}


int usbd_read(uint8_t* buf, uint32_t len)
{
	uint32_t timeout = 0xffffff;
	int count = 0;
	if(rxdata_count == 0)
	{
	        rx_completed = 0;
                usbd_rx_prepare();
		/*while (!rx_completed && timeout != 0 && rxdata_count == 0) {
			--timeout;
		}*/
	}

	if(rxdata_count > 0)
	{
	    count = (len < rxdata_count)? len: rxdata_count;
	    usbd_rx_datacopy(buf, count);
	    rxdata_count -= count;
	    rxdata_index += count;
	}

	return count;
}


int usbd_install(const struct usbd_config *config)
{

	vendor_rx_ep = MSS_USB_RX_EP_1;
	vendor_tx_ep = MSS_USB_TX_EP_2;
	rx_completed = 0;
	rxdata_count = 0;
	rxdata_index = 0;

	/* Initialize USB driver. */
	MSS_USBD_init(MSS_USB_DEVICE_HS);

	/*Assign call-back function Interface needed by USBD driver*/
	MSS_USBD_set_descr_cb_handler(&vendor_dev_descriptors_cb);

	/*Initialize the USBD_VENDOR class driver*/
	MSS_USBD_VENDOR_init(MSS_USB_DEVICE_HS);

	return 0;
}