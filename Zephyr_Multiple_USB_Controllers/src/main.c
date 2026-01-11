#include <zephyr/kernel.h>
#include <zephyr/usb/usbd.h>

#define USB_VID_ZEPHYR 0x2fe3
#define USB_PID_PRODUCT 0xcaca

USBD_DEVICE_DEFINE(test_usbd0, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)), USB_VID_ZEPHYR, USB_PID_PRODUCT);
USBD_DEVICE_DEFINE(test_usbd1, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc1)), USB_VID_ZEPHYR, USB_PID_PRODUCT);

int main(void)
{
	printk("Test.\n");

	k_sleep(K_FOREVER);

	return 0;
}
