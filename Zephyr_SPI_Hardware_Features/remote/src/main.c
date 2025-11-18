#include <hal/nrf_ipct.h>
#include <nrfx_dppi.h>
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
#endif

#define DPPI_CHANNEL 4

ISR_DIRECT_DECLARE(timerInterruptHandler)
{
    // Invoke the handler at nRFx driver level, which in turn will invoke the event handler registered below
    NRFX_TIMER_INST_HANDLER_GET(TIMER_INSTANCE_ID)();
    ISR_DIRECT_PM();
    return 1;
}

static void timerHandler(nrf_timer_event_t event_type, void *p_context)
{
	static uint32_t counter = 0;

	printk("CIAO %u.\n", counter);
	counter++;
}

static void configureDPPI(void)
{
	uint32_t eventEndpoint;
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
	#endif
}

int main(void)
{
	nrfx_timer_t timerInstance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_ID);
	nrfx_timer_config_t timerConfiguration = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	uint32_t ticks;
	nrfx_err_t ret;

	LOG_INF("Radio core is starting.");

	// Initialize the selected timer
	timerConfiguration.bit_width = NRF_TIMER_BIT_WIDTH_32;
	ret = nrfx_timer_init(&timerInstance, &timerConfiguration, timerHandler);
	if (ret != NRFX_SUCCESS) {
		LOG_ERR("Error : failed to initialize the timer (0x%08X).", ret);
		return -1;
	}
	nrfx_timer_clear(&timerInstance);

	// Configure the timer channel 0 to overflow each second
	IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID)), DT_IRQ(DT_NODELABEL(TIMER_DT_NODE_LABEL), priority), timerInterruptHandler, 0);
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
