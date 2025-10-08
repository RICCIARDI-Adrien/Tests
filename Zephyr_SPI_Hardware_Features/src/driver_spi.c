#include <nrfx_spim.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(driver_spi, LOG_LEVEL_DBG);

// SPIM buffers must be in Data RAM for the EasyDMA to be able to access them
static unsigned char __attribute__((__section__("DMA_RAM3x_APP"))) __attribute__((aligned(sizeof(uint32_t)))) transmission_buffer[16];
static unsigned char __attribute__((__section__("DMA_RAM3x_APP"))) __attribute__((aligned(sizeof(uint32_t)))) reception_buffer[16];

struct driver_spi_config {
	nrfx_spim_t spim_instance;
	const struct pinctrl_dev_config *pcfg;
};

static void driver_spi_handler(nrfx_spim_evt_t const * p_event, void * p_context)
{
    LOG_DBG("Interrupt.");
}

static int driver_spi_init(const struct device *dev)
{
	const struct driver_spi_config *config = dev->config;
	nrfx_spim_config_t spim_config = {
		.use_hw_ss = true,
		.ss_duration = 30,
		.ss_active_high = false,
		.irq_priority = NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY,
		.frequency = MHZ(8),
		.mode = NRF_SPIM_MODE_0,
		.bit_order = NRF_SPIM_BIT_ORDER_MSB_FIRST,
		.miso_pull = NRF_GPIO_PIN_NOPULL,
		.skip_gpio_cfg = true,
		.skip_psel_cfg = true
	};
	nrfx_err_t ret;

	LOG_DBG("Starting initialization.");

	// TEST
	IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_SPIM_INST_GET(131)), IRQ_PRIO_LOWEST, NRFX_SPIM_INST_HANDLER_GET(131), 0, 0);

	ret = nrfx_spim_init(&config->spim_instance, &spim_config, driver_spi_handler, NULL);
    if (ret != NRFX_SUCCESS) {
		LOG_ERR("Failed to initialize the SPIM peripheral.");
		return ret;
	}

	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret != 0) {
		LOG_ERR("Failed to apply the default pin control state.");
		return ret;
	}

	return 0;
}

static int driver_spi_transceive(const struct device *dev, const struct spi_config *spi_cfg, const struct spi_buf_set *tx_bufs, const struct spi_buf_set *rx_bufs)
{
	const struct driver_spi_config *config = dev->config;
	nrfx_err_t ret;

	LOG_DBG("Starting transferring data.");

	// TEST
	memcpy(transmission_buffer, "SALUT !", 7);
	nrfx_spim_xfer_desc_t transfer_descriptor = NRFX_SPIM_XFER_TRX(transmission_buffer, 7, reception_buffer, 7);
	ret = nrfx_spim_xfer(&config->spim_instance, &transfer_descriptor, 0);
	if (ret != NRFX_SUCCESS) {
		LOG_ERR("SPI transfer failed (0x%X).", ret);
		return ret;
	}

	return 0;
}

static DEVICE_API(spi, driver_spi_api) = {
	.transceive = driver_spi_transceive
};

#define SPIM(index) DT_NODELABEL(spi##index)

#define DRIVER_SPI_DEFINE(periph_name, prefix, index, ...) \
	PINCTRL_DT_DEFINE(SPIM(index)); \
	static const struct driver_spi_config driver_spi_##index##_config = { \
		.spim_instance = { \
			.p_reg = (NRF_SPIM_Type *) DT_REG_ADDR(SPIM(index)), \
			.drv_inst_idx = NRFX_SPIM##index##_INST_IDX \
		}, \
		.pcfg = PINCTRL_DT_DEV_CONFIG_GET(SPIM(index)) \
	}; \
	SPI_DEVICE_DT_DEFINE(SPIM(index), driver_spi_init, NULL, NULL, &driver_spi_##index##_config, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &driver_spi_api);
	
NRFX_FOREACH_ENABLED(SPIM, DRIVER_SPI_DEFINE, (), ())
