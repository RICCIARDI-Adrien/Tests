#include <zephyr/kernel.h>

int main(void)
{
	printk("Radio core is sleeping forever.\n");
	
	k_sleep(K_FOREVER);
	return 0;
}
