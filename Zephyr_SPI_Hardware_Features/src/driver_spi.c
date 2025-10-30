#include <helpers/nrfx_gppi.h>
#include <nrfx_spim.h>
#include <nrfx_timer.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(driver_spi, LOG_LEVEL_DBG);

#define TIMER_INSTANCE_ID 132
#define TIMER_DT_NODE_LABEL timer132

#define SPIM_INSTANCE_ID 131
#define SPIM_DT_NODE_LABEL spi131

#define DPPI_CHANNEL 4

struct driver_spi_config {
	nrfx_spim_t spim_instance;
	const struct pinctrl_dev_config *pcfg;
};

// SPIM buffers must be in Data RAM for the EasyDMA to be able to access them
#ifdef NRF_RADIOCORE
	#define BUFFERS_SECTION "DMA_RAM3x_RAD"
#else
	#define BUFFERS_SECTION "DMA_RAM3x_APP"
#endif
static unsigned char __attribute__((__section__(BUFFERS_SECTION))) __attribute__((aligned(sizeof(uint32_t)))) transmission_buffer[16];
static unsigned char __attribute__((__section__(BUFFERS_SECTION))) __attribute__((aligned(sizeof(uint32_t)))) reception_buffer[16];

static nrfx_spim_xfer_desc_t transfer_descriptor = NRFX_SPIM_XFER_TRX(transmission_buffer, 7, reception_buffer, 7);

static NRF_TIMER_Type *pointerTimer = (NRF_TIMER_Type *) DT_REG_ADDR(DT_NODELABEL(TIMER_DT_NODE_LABEL));
static NRF_SPIM_Type *pointerSPIM = (NRF_SPIM_Type *) DT_REG_ADDR(DT_NODELABEL(SPIM_DT_NODE_LABEL));

static void driver_spi_handler(nrfx_spim_evt_t const * p_event, void * p_context)
{
    LOG_DBG("Interrupt.");
}

static void configureDppi(void)
{
	uint32_t eventEndpoint, taskEndpoint;

	eventEndpoint = nrf_timer_event_address_get(pointerTimer, NRF_TIMER_EVENT_COMPARE0);
	taskEndpoint = nrf_spim_task_address_get(pointerSPIM, NRF_SPIM_TASK_START);

	// The high-level way
	#if 0
	{
		uint8_t gppiChannel;
		nrfx_err_t ret;

		ret = nrfx_gppi_channel_alloc(&gppiChannel);
		if (ret != NRFX_SUCCESS) {
			LOG_ERR("Failed to allocate a GPPI channel.");
			return;
		}
		LOG_DBG("Allocated GPPI channel : %u.", gppiChannel);

		nrfx_gppi_channel_endpoints_setup(gppiChannel, eventEndpoint, taskEndpoint);
		nrfx_gppi_channels_enable(NRFX_BIT(gppiChannel));
	}
	// The low-level way
	#else
	{
		NRF_DPPIC_Type *pointerDPPIC;

		NRF_DPPI_ENDPOINT_SETUP(eventEndpoint, DPPI_CHANNEL);
		NRF_DPPI_ENDPOINT_SETUP(taskEndpoint, DPPI_CHANNEL);

		// Enable the channel
		// DPPIC134
		pointerDPPIC = (NRF_DPPIC_Type *) DT_REG_ADDR(DT_NODELABEL(dppic134));
		nrf_dppi_channels_enable(pointerDPPIC, 1 << DPPI_CHANNEL);
		// DPPIC130
		pointerDPPIC = (NRF_DPPIC_Type *) DT_REG_ADDR(DT_NODELABEL(dppic130));
		nrf_dppi_channels_enable(pointerDPPIC, 1 << DPPI_CHANNEL);
		// DPPIC133
		pointerDPPIC = (NRF_DPPIC_Type *) DT_REG_ADDR(DT_NODELABEL(dppic133));
		nrf_dppi_channels_enable(pointerDPPIC, 1 << DPPI_CHANNEL);
	}
	#endif
}

static int driver_spi_init(const struct device *dev)
{
	const struct driver_spi_config *config = dev->config;
	nrfx_spim_config_t spim_config = {
		.use_hw_ss = true,
		.ss_duration = 30,
		.ss_active_high = false,
		.irq_priority = NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY,
		.frequency = MHZ(2), // Decrease the frequency to better see the DK LEDs connected to the SPI pins flashing
		.mode = NRF_SPIM_MODE_0,
		.bit_order = NRF_SPIM_BIT_ORDER_MSB_FIRST,
		.miso_pull = NRF_GPIO_PIN_NOPULL,
		.skip_gpio_cfg = true,
		.skip_psel_cfg = false // There is a bug in the NRF API that skips setting the hardware /CS feature if PSELs are not configured
	};
	nrfx_err_t ret;
	nrfx_timer_t timerInstance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_ID);
	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	uint32_t ticks;

	LOG_DBG("Starting initialization.");

	// TEST
#ifdef NRF_RADIOCORE
	IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_SPIM_INST_GET(132)), IRQ_PRIO_LOWEST, NRFX_SPIM_INST_HANDLER_GET(132), 0, 0);
#else
	IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_SPIM_INST_GET(SPIM_INSTANCE_ID)), IRQ_PRIO_LOWEST, NRFX_SPIM_INST_HANDLER_GET(SPIM_INSTANCE_ID), 0, 0);
#endif

	// Initialize the selected timer
	timer_config.bit_width = NRF_TIMER_BIT_WIDTH_32;
	ret = nrfx_timer_init(&timerInstance, &timer_config, NULL);
	if (ret != NRFX_SUCCESS) {
		printk("Error : failed to initialize the timer (%d).", ret);
		return -1;
	}
	nrfx_timer_clear(&timerInstance);

	// Configure the timer channel 0 to overflow each 200ms
	ticks = nrfx_timer_us_to_ticks(&timerInstance, 200000);
	nrfx_timer_extended_compare(&timerInstance, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);

	ret = nrfx_spim_init(&config->spim_instance, &spim_config, driver_spi_handler, NULL);
    if (ret != NRFX_SUCCESS) {
		LOG_ERR("Failed to initialize the SPIM peripheral.");
		return ret;
	}

	memcpy(transmission_buffer, "SALUT !", 7);
	nrfy_spim_buffers_set(pointerSPIM, &transfer_descriptor);
	nrfy_spim_int_enable(pointerSPIM, NRF_SPIM_INT_END_MASK);
	nrfy_spim_enable(pointerSPIM);

	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret != 0) {
		LOG_ERR("Failed to apply the default pin control state.");
		return ret;
	}

	configureDppi();

	// Start ticking the SPI transfers
	nrfx_timer_enable(&timerInstance);

	return 0;
}

static int driver_spi_transceive(const struct device *dev, const struct spi_config *spi_cfg, const struct spi_buf_set *tx_bufs, const struct spi_buf_set *rx_bufs)
{
	/*const struct driver_spi_config *config = dev->config;
	nrfx_err_t ret;

	LOG_DBG("Starting transferring data.");

	// TEST
	memcpy(transmission_buffer, "SALUT !", 7);
	nrfx_spim_xfer_desc_t transfer_descriptor = NRFX_SPIM_XFER_TRX(transmission_buffer, 7, reception_buffer, 7);
	ret = nrfx_spim_xfer(&config->spim_instance, &transfer_descriptor, 0);
	if (ret != NRFX_SUCCESS) {
		LOG_ERR("SPI transfer failed (0x%X).", ret);
		return ret;
	}*/
	printk("Fake transfer from main thread.\n");

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
