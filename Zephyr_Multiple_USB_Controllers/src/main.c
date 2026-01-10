#include <zephyr/kernel.h>

int main(void)
{
	printk("Test.\n");

	k_sleep(K_FOREVER);

	return 0;
}
