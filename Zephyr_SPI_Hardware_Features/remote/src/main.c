#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>

static const struct device *spi_device = DEVICE_DT_GET(DT_NODELABEL(spi132));

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
	struct spi_config spi_config = {
		.operation = SPI_OP_MODE_MASTER | SPI_LINES_SINGLE | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8)
	};

	printk("Radio core is starting.\n");

	while (1) {
		memcpy(buffer, "SALUT !", 7);
		ret = spi_write(spi_device, &spi_config, &buffers_set);

		k_msleep(1000);
	}

	return 0;
}
