#include <zephyr/kernel.h>
#include <zephyr/usb/usbd.h>

#define USB_VID_ZEPHYR 0x2FE3

typedef struct
{
	struct usbd_desc_node *Pointer_Descriptor_String_Language;
	struct usbd_desc_node *Pointer_Descriptor_String_Manufacturer;
	struct usbd_desc_node *Pointer_Descriptor_String_Product;
	struct usbd_config_node *Pointer_Descriptor_Configuration;
	const char *Pointer_String_Class_Name;
} TUSBConfigurationData;

USBD_DEVICE_DEFINE(USB_Context_0, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)), USB_VID_ZEPHYR, 0xFADA);
USBD_DEVICE_DEFINE(USB_Context_1, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc1)), USB_VID_ZEPHYR, 0xBEEF);

USBD_DESC_LANG_DEFINE(USB_Descriptor_Language_0);
USBD_DESC_MANUFACTURER_DEFINE(USB_Descriptor_String_Manufacturer_0, "Saucisson");
USBD_DESC_PRODUCT_DEFINE(USB_Descriptor_String_Product_0, "Fromage");
USBD_DESC_CONFIG_DEFINE(USB_Descriptor_String_Configuration_0, "Premiere config");
USBD_CONFIGURATION_DEFINE(USB_Configuration_0, 0, 100, &USB_Descriptor_String_Configuration_0);

USBD_DESC_LANG_DEFINE(USB_Descriptor_Language_1);
USBD_DESC_MANUFACTURER_DEFINE(USB_Descriptor_String_Manufacturer_1, "Vin");
USBD_DESC_PRODUCT_DEFINE(USB_Descriptor_String_Product_1, "Armagnac");
USBD_DESC_CONFIG_DEFINE(USB_Descriptor_String_Configuration_1, "Deuxieme config");
USBD_CONFIGURATION_DEFINE(USB_Configuration_1, 0, 100, &USB_Descriptor_String_Configuration_1);

static TUSBConfigurationData USB_Configuration_Datas[] =
{
	// Configuration 0
	{
		.Pointer_Descriptor_String_Language = &USB_Descriptor_Language_0,
		.Pointer_Descriptor_String_Manufacturer = &USB_Descriptor_String_Manufacturer_0,
		.Pointer_Descriptor_String_Product = &USB_Descriptor_String_Product_0,
		.Pointer_Descriptor_Configuration = &USB_Configuration_0,
		.Pointer_String_Class_Name = "cdc_acm_0"
	},
	// Configuration 1
	{
		.Pointer_Descriptor_String_Language = &USB_Descriptor_Language_1,
		.Pointer_Descriptor_String_Manufacturer = &USB_Descriptor_String_Manufacturer_1,
		.Pointer_Descriptor_String_Product = &USB_Descriptor_String_Product_1,
		.Pointer_Descriptor_Configuration = &USB_Configuration_1,
		.Pointer_String_Class_Name = "cdc_acm_1"
	}
};

static int ConfigureUSB(struct usbd_context *Pointer_USB_Context, TUSBConfigurationData *Pointer_Configuration_Data)
{
	int Result;

	Result = usbd_add_descriptor(Pointer_USB_Context, Pointer_Configuration_Data->Pointer_Descriptor_String_Language);
	if (Result != 0)
	{
		printk("Failed to initialize language descriptor (%d).", Result);
		return -1;
	}

	Result = usbd_add_descriptor(Pointer_USB_Context, Pointer_Configuration_Data->Pointer_Descriptor_String_Manufacturer);
	if (Result != 0)
	{
		printk("Failed to initialize manufacturer descriptor (%d).", Result);
		return -1;
	}

	Result = usbd_add_descriptor(Pointer_USB_Context, Pointer_Configuration_Data->Pointer_Descriptor_String_Product);
	if (Result != 0)
	{
		printk("Failed to initialize product descriptor (%d).", Result);
		return -1;
	}

	Result = usbd_add_configuration(Pointer_USB_Context, USBD_SPEED_FS, Pointer_Configuration_Data->Pointer_Descriptor_Configuration);
	if (Result != 0)
	{
		printk("Failed to add Full-Speed configuration (%d).", Result);
		return -1;
	}

	// Register the class belonging to each USB controller
	Result = usbd_register_class(Pointer_USB_Context, Pointer_Configuration_Data->Pointer_String_Class_Name, USBD_SPEED_FS, 1);
	if (Result != 0)
	{
		printk("Failed to register the class \"%s\" to the controller \"%s\" (%d).", Pointer_Configuration_Data->Pointer_String_Class_Name, Pointer_USB_Context->name, Result);
		return -1;
	}

	Result = usbd_init(Pointer_USB_Context);
	if (Result != 0)
	{
		printk("Failed to initialize the UDC \"%s\" device support (%d).", Pointer_USB_Context->name, Result);
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

	if (ConfigureUSB(&USB_Context_0, &USB_Configuration_Datas[0]) != 0)
	{
		printk("Error : USB device 0 configuration failed.");
		return -1;
	}

	if (ConfigureUSB(&USB_Context_1, &USB_Configuration_Datas[1]) != 0)
	{
		printk("Error : USB device 1 configuration failed.");
		return -1;
	}

	printk("Waiting forever.");
	k_sleep(K_FOREVER);

	return 0;
}
