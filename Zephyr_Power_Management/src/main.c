#include <zephyr/kernel.h>

int main(void)
{
	printk("Board %s has booted and is going to suspend to idle.\n", CONFIG_BOARD_TARGET);

	k_sleep(K_FOREVER);
	return 0;
}
