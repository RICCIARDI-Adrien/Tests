#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>

static const struct device *spi_device = DEVICE_DT_GET(DT_NODELABEL(spi131));

int main(void)
{
	int ret;
	uint8_t buffer[16];
	struct spi_buf buffers_descriptor = {
		.buf = buffer
	};
	struct spi_buf_set buffers_set = {
		.buffers = &buffers_descriptor,
		.count = 1
	};
	struct spi_config spi_config = {0};

	printk("App core is starting.\n");

	while (1) {
		memcpy(buffer, "SALUT !", 7);
		ret = spi_write(spi_device, &spi_config, &buffers_set);

		k_msleep(1000);
	}

	return 0;
}
