#include <zephyr/kernel.h>
#include <zephyr/usb/usbd.h>

#define USB_VID_ZEPHYR 0x2fe3
#define USB_PID_PRODUCT 0xcaca

USBD_DEVICE_DEFINE(USB_Context_0, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)), USB_VID_ZEPHYR, USB_PID_PRODUCT);

USBD_DESC_LANG_DEFINE(USB_Descriptor_Language);
USBD_DESC_MANUFACTURER_DEFINE(USB_Descriptor_String_Manufacturer, "Saucisson");
USBD_DESC_PRODUCT_DEFINE(USB_Descriptor_String_Product, "Fromage");

USBD_DESC_CONFIG_DEFINE(USB_Descriptor_Configuration, "FS Configuration");

USBD_CONFIGURATION_DEFINE(USB_Configuration, 0, 100, &USB_Descriptor_Configuration);

int ConfigureUSB(struct usbd_context *Pointer_USB_Context)
{
	int Result;

	Result = usbd_add_descriptor(Pointer_USB_Context, &USB_Descriptor_Language);
	if (Result != 0)
	{
		printk("Failed to initialize language descriptor (%d).", Result);
		return -1;
	}

	Result = usbd_add_descriptor(Pointer_USB_Context, &USB_Descriptor_String_Manufacturer);
	if (Result != 0)
	{
		printk("Failed to initialize manufacturer descriptor (%d).", Result);
		return -1;
	}

	Result = usbd_add_descriptor(Pointer_USB_Context, &USB_Descriptor_String_Product);
	if (Result != 0)
	{
		printk("Failed to initialize product descriptor (%d).", Result);
		return -1;
	}

	Result = usbd_add_configuration(Pointer_USB_Context, USBD_SPEED_FS, &USB_Configuration);
	if (Result != 0)
	{
		printk("Failed to add Full-Speed configuration (%d).", Result);
		return -1;
	}

	Result = usbd_register_all_classes(Pointer_USB_Context, USBD_SPEED_FS, 1, NULL);
	if (Result != 0)
	{
		printk("Failed to register all classes (%d).", Result);
		return -1;
	}

	Result = usbd_init(Pointer_USB_Context);
	if (Result != 0)
	{
		printk("Failed to initialize the UDC0 device support.");
		return -1;
	}

	Result = usbd_enable(Pointer_USB_Context);
	if (Result != 0)
	{
		printk("Failed to enable device (%d).", Result);
		return -1;
	}

	return 0;
}

int main(void)
{
	printk("Starting application.\n");

	if (ConfigureUSB(&USB_Context_0) != 0)
	{
		printk("Error : USB device 0 configuration failed.");
		return -1;
	}

	printk("Waiting forever.");
	k_sleep(K_FOREVER);

	return 0;
}
