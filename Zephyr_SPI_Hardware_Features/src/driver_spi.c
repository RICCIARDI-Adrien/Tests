#include <nrfx_spim.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(driver_spi, LOG_LEVEL_DBG);

struct driver_spi_config {
	const struct pinctrl_dev_config *pcfg;
};

static int driver_spi_init(const struct device *dev)
{
	LOG_DBG("Starting initialization.");

	return 0;
}

static int driver_spi_transceive(const struct device *dev, const struct spi_config *spi_cfg, const struct spi_buf_set *tx_bufs, const struct spi_buf_set *rx_bufs)
{
	return 0;
}

static DEVICE_API(spi, driver_spi_api) = {
	.transceive = driver_spi_transceive
};

#define SPIM(index) DT_NODELABEL(spi##index)

#define DRIVER_SPI_DEFINE(periph_name, prefix, index, ...) \
	PINCTRL_DT_DEFINE(SPIM(index)); \
	static const struct driver_spi_config driver_spi_##index##_config = { \
		.pcfg = PINCTRL_DT_DEV_CONFIG_GET(SPIM(index)) \
	}; \
	SPI_DEVICE_DT_DEFINE(SPIM(index), driver_spi_init, NULL, NULL, &driver_spi_##index##_config, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &driver_spi_api);
	
NRFX_FOREACH_ENABLED(SPIM, DRIVER_SPI_DEFINE, (), ())
