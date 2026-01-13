#include <hal/nrf_ipct.h>
#include <helpers/nrfx_gppi.h>
#include <nrfx_timer.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(radiocore, LOG_LEVEL_DBG);

#ifdef USE_TIMER132
	#define TIMER_DT_NODE_LABEL timer132
	#define TIMER_INSTANCE_ID 132

	#define DPPIC_DT_NODE_LABEL dppic134
#else
	#define TIMER_DT_NODE_LABEL timer022
	#define TIMER_INSTANCE_ID 022

	#define DPPIC_DT_NODE_LABEL dppic020

	#define IPCT_DT_NODE_LABEL cpurad_ipct
#endif

#define DPPI_CHANNEL 4

static nrfx_timer_t timerInstance = NRFX_TIMER_INSTANCE(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID));
static NRF_IPCT_Type *ipctRegisters = (NRF_IPCT_Type *) DT_REG_ADDR(DT_NODELABEL(IPCT_DT_NODE_LABEL));

static void timerInterruptHandler(nrf_timer_event_t event_type, void *p_context)
{
	static uint32_t counter = 0;

	printk("CIAO %u.\n", counter);
	counter++;
}

static void configureDPPI(void)
{
	/*uint32_t eventEndpoint;
	NRF_DPPIC_Type *pointerDPPIC;
	NRF_TIMER_Type *pointerTimer = (NRF_TIMER_Type *) DT_REG_ADDR(DT_NODELABEL(TIMER_DT_NODE_LABEL));

	eventEndpoint = nrf_timer_event_address_get(pointerTimer, NRF_TIMER_EVENT_COMPARE0);
	NRF_DPPI_ENDPOINT_SETUP(eventEndpoint, DPPI_CHANNEL);

	pointerDPPIC = (NRF_DPPIC_Type *) DT_REG_ADDR(DT_NODELABEL(DPPIC_DT_NODE_LABEL));
	nrf_dppi_channels_enable(pointerDPPIC, 1 << DPPI_CHANNEL);

	// Connect the IPCT channel to the DPPI channel when needed
	#ifndef USE_TIMER132
	{
		NRF_IPCT_Type *pointerIPCT = (NRF_IPCT_Type *) DT_REG_ADDR(DT_NODELABEL(cpurad_ipct));

		nrf_ipct_subscribe_set(pointerIPCT, NRF_IPCT_TASK_SEND_0, DPPI_CHANNEL);
	}
	#endif*/

	#ifdef USE_TIMER132
		#error "TODO"
	#else
	{
		uint32_t sourceEndpoint, destinationEndpoint;
		nrfx_gppi_handle_t gppiHandle;
		int ret;

		sourceEndpoint = nrf_timer_event_address_get(timerInstance.p_reg, NRF_TIMER_EVENT_COMPARE0);
		destinationEndpoint = nrf_ipct_task_address_get(ipctRegisters, NRF_IPCT_TASK_SEND_0);
	}
	#endif
}

int main(void)
{
	nrfx_timer_config_t timerConfiguration = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	uint32_t ticks;
	int ret;

	LOG_INF("Radio core is starting.");

	// Initialize the selected timer
	timerConfiguration.bit_width = NRF_TIMER_BIT_WIDTH_32;
	ret = nrfx_timer_init(&timerInstance, &timerConfiguration, timerInterruptHandler);
	if (ret != 0) {
		LOG_ERR("Error : failed to initialize the timer (0x%08X).", ret);
		return -1;
	}
	nrfx_timer_clear(&timerInstance);

	// Configure the timer channel 0 to overflow each second
	IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID)), DT_IRQ(DT_NODELABEL(TIMER_DT_NODE_LABEL), priority), nrfx_timer_irq_handler, &timerInstance, 0);
	ticks = nrfx_timer_us_to_ticks(&timerInstance, 1000000);
	nrfx_timer_extended_compare(&timerInstance, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

	// Configure the DPPI connections
	configureDPPI();

	// Start ticking the various peripherals connected to the timer channel
	nrfx_timer_enable(&timerInstance);

	LOG_INF("Radio core is sleeping forever.");
	k_sleep(K_FOREVER);
	return 0;
}
