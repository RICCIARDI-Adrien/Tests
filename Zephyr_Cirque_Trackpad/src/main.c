#include <zephyr/kernel.h>

int main(void)
{
	printk("Starting application.\n");

	k_sleep(K_FOREVER);

	return 0;
}
