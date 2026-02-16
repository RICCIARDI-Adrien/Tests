#include <hal/nrf_ipct.h>
#include <helpers/nrfx_gppi.h>
#include <nrfx_spim.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

#include "driver_spi.h"

LOG_MODULE_REGISTER(driver_spi, LOG_LEVEL_DBG);

// Use the official driver device tree definitions by stealing its name
#define DT_DRV_COMPAT nordic_nrf_spim

#define SPIM_DT_NODE_LABEL spi131

#define DPPI_CHANNEL 4

#ifndef USE_TIMER132
	#define IPCT_DT_NODE_LABEL ipct130
#endif

struct driver_spi_config {
	void (*irq_connect)(void);
	const struct pinctrl_dev_config *pcfg;
};

struct driver_spi_data {
	nrfx_spim_t spim_instance;
};

// SPIM buffers must be in Data RAM for the EasyDMA to be able to access them
#define BUFFERS_SECTION "DMA_RAM3x_APP"
static unsigned char __attribute__((__section__(BUFFERS_SECTION))) __attribute__((aligned(sizeof(uint32_t)))) transmission_buffer[16];
static unsigned char __attribute__((__section__(BUFFERS_SECTION))) __attribute__((aligned(sizeof(uint32_t)))) reception_buffer[16];

static nrfx_spim_xfer_desc_t transfer_descriptor = NRFX_SPIM_XFER_TRX(transmission_buffer, 7, reception_buffer, 7);

static NRF_SPIM_Type *pointerSPIM = (NRF_SPIM_Type *) DT_REG_ADDR(DT_NODELABEL(SPIM_DT_NODE_LABEL));

#ifndef USE_TIMER132
static NRF_IPCT_Type *ipctRegisters = (NRF_IPCT_Type *) DT_REG_ADDR(DT_NODELABEL(IPCT_DT_NODE_LABEL));
#endif

static void driver_spi_handler(nrfx_spim_event_t const * p_event, void * p_context)
{
    LOG_DBG("Interrupt.");
}

static void configureDppi(const struct device *dev)
{
#if 0
	uint32_t taskEndpoint;
	NRF_DPPIC_Type *pointerDPPIC;

	taskEndpoint = nrf_spim_task_address_get(pointerSPIM, NRF_SPIM_TASK_START);

	NRF_DPPI_ENDPOINT_SETUP(taskEndpoint, DPPI_CHANNEL);

	// Enable the channel
	// DPPIC130
	pointerDPPIC = (NRF_DPPIC_Type *) DT_REG_ADDR(DT_NODELABEL(dppic130));
	nrf_dppi_channels_enable(pointerDPPIC, 1 << DPPI_CHANNEL);
	// DPPIC133
	pointerDPPIC = (NRF_DPPIC_Type *) DT_REG_ADDR(DT_NODELABEL(dppic133));
	nrf_dppi_channels_enable(pointerDPPIC, 1 << DPPI_CHANNEL);

	enableDPPIPolling(false);
#endif


	#ifdef USE_TIMER132
		#error "TODO"
	#else
		struct driver_spi_data *data = dev->data;
		uint32_t sourceEndpoint, destinationEndpoint;
		nrfx_gppi_handle_t gppiHandle;
		int ret;

		sourceEndpoint = nrf_ipct_event_address_get(ipctRegisters, NRF_IPCT_EVENT_RECEIVE_0 + (DPPI_CHANNEL * 4));
		destinationEndpoint = nrf_spim_task_address_get(data->spim_instance.p_reg, NRF_SPIM_TASK_START);

		ret = nrfx_gppi_conn_alloc(sourceEndpoint, destinationEndpoint, &gppiHandle);
		if (ret != 0)
		{
			printk("Error : failed to allocated a GPPI connection (0x%08X).\n", ret);
			return;
		}

		// The receiver needs to acknowledge each event, offload this to the hardware
		nrf_ipct_shorts_enable(ipctRegisters, 1 << 4);

		nrfx_gppi_conn_enable(gppiHandle);
	#endif
}

static int driver_spi_init(const struct device *dev)
{
	const struct driver_spi_config *config = dev->config;
	struct driver_spi_data *data = dev->data;
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
	int ret;

	LOG_DBG("Starting initialization.");

	ret = nrfx_spim_init(&data->spim_instance, &spim_config, driver_spi_handler, NULL);
	if (ret != 0) {
		LOG_ERR("Failed to initialize the SPIM peripheral.");
		return ret;
	}

	config->irq_connect();

	memcpy(transmission_buffer, "SALUT !", 7);
	nrfy_spim_buffers_set(pointerSPIM, &transfer_descriptor);
	nrfy_spim_int_enable(pointerSPIM, NRF_SPIM_INT_END_MASK);
	nrfy_spim_enable(pointerSPIM);

	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret != 0) {
		LOG_ERR("Failed to apply the default pin control state.");
		return ret;
	}

	configureDppi(dev);

	LOG_DBG("Initialization succeeded.");

	return 0;
}

static int driver_spi_transceive(const struct device *dev, const struct spi_config *spi_cfg, const struct spi_buf_set *tx_bufs, const struct spi_buf_set *rx_bufs)
{
#if 0
	struct driver_spi_data *data = dev->data;
	int ret;

	LOG_DBG("Starting transferring data.");

	// TEST
	memcpy(transmission_buffer, "SALUT !", 7);
	nrfx_spim_xfer_desc_t transfer_descriptor = NRFX_SPIM_XFER_TRX(transmission_buffer, 7, reception_buffer, 7);
	ret = nrfx_spim_xfer(&data->spim_instance, &transfer_descriptor, 0);
	if (ret != 0) {
		LOG_ERR("SPI transfer failed (0x%X).", ret);
		return ret;
	}
#else
	ARG_UNUSED(dev);
	ARG_UNUSED(spi_cfg);
	ARG_UNUSED(tx_bufs);
	ARG_UNUSED(rx_bufs);

	printk("Fake transfer from main thread.\n");
#endif

	return 0;
}

void enableDPPIPolling(bool isEnabled)
{
	if (isEnabled)
		pointerSPIM->SUBSCRIBE_START |= 0x80000000;
	else
		pointerSPIM->SUBSCRIBE_START &= 0x7FFFFFFF;
}

static DEVICE_API(spi, driver_spi_api) = {
	.transceive = driver_spi_transceive
};

#define DRIVER_SPI_DEFINE(inst) \
	PINCTRL_DT_INST_DEFINE(inst); \
	static struct driver_spi_data driver_spi_##inst##_data = { \
		.spim_instance = NRFX_SPIM_INSTANCE(DT_INST_REG_ADDR(inst)) \
	}; \
	static void driver_spi_##inst##_irq_connect(void) \
	{ \
		IRQ_CONNECT(DT_INST_IRQN(inst), DT_INST_IRQ(inst, priority), nrfx_spim_irq_handler, &driver_spi_##inst##_data.spim_instance, 0); \
	} \
	static const struct driver_spi_config driver_spi_##inst##_config = { \
		.irq_connect = driver_spi_##inst##_irq_connect, \
		.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(inst) \
	}; \
	SPI_DEVICE_DT_INST_DEFINE(inst, driver_spi_init, NULL, &driver_spi_##inst##_data, &driver_spi_##inst##_config, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &driver_spi_api);
	
DT_INST_FOREACH_STATUS_OKAY(DRIVER_SPI_DEFINE)
