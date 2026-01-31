#include <zephyr/input/input.h>
#include <zephyr/kernel.h>

static void TrackpadInputCallback(struct input_event *evt, void *user_data)
{
	printk("code=%u, val=%d\n", evt->code, evt->value);
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(trackpad)), TrackpadInputCallback, NULL);

int main(void)
{
	printk("Starting application.\n");

	k_sleep(K_FOREVER);

	return 0;
}
