#include <helpers/nrfx_gppi.h>
#include <nrfx_gpiote.h>
#include <nrfx_timer.h>
#include <zephyr/kernel.h>

#define TIMER_DT_LABEL timer132
#define TIMER_INSTANCE_ID 132
#define GPIOTE_INSTANCE_ID 130

// Port 9 pin 0
#define GPIO_PIN (9 << 5)

ISR_DIRECT_DECLARE(timer_interrupt_handler)
{
    // Invoke the handler at nRFx driver level, which in turn will invoke the event handler registered below
    NRFX_TIMER_INST_HANDLER_GET(TIMER_INSTANCE_ID)();
    ISR_DIRECT_PM();
    return 1;
}

static void timerInterruptHandler(nrf_timer_event_t event_type, void *p_context)
{
	static uint32_t counter = 0;

	printk("CIAO %u.\n", counter);
	counter++;
}

static void configureDppi(void)
{
	uint8_t gppiChannel;
	uint32_t sourceEndpoint, destinationEndpoint;
	nrfx_timer_t timerInstance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_ID);
	nrfx_gpiote_t gpioteInstance = NRFX_GPIOTE_INSTANCE(GPIOTE_INSTANCE_ID);
	nrfx_err_t ret;

	sourceEndpoint = nrfx_timer_event_address_get(&timerInstance, NRF_TIMER_EVENT_COMPARE0);
	destinationEndpoint = nrfx_gpiote_out_task_address_get(&gpioteInstance, GPIO_PIN);

	ret = nrfx_gppi_channel_alloc(&gppiChannel);
	if (ret != NRFX_SUCCESS)
	{
		printk("Error : failed to allocated a GPPI channel (0x%08X).\n", ret);
		return;
	}
	printk("Allocated GPPI channel : %u.\n", gppiChannel);

	nrfx_gppi_channel_endpoints_setup(gppiChannel, sourceEndpoint, destinationEndpoint);
	nrfx_gppi_channels_enable(NRFX_BIT(gppiChannel));
}

int main(void)
{
	nrfx_err_t ret;
	nrfx_timer_config_t timerConfiguration = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	nrfx_timer_t timerInstance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_ID);
	uint32_t ticks;
	static nrfx_gpiote_t gpioteInstance = NRFX_GPIOTE_INSTANCE(GPIOTE_INSTANCE_ID);
	nrfx_gpiote_output_config_t gpioteOutputConfiguration =
	{
		.drive = NRF_GPIO_PIN_S0S1,
		.input_connect = NRF_GPIO_PIN_INPUT_DISCONNECT,
		.pull = NRF_GPIO_PIN_NOPULL
	};
	nrfx_gpiote_task_config_t gpioteTaskConfiguration =
	{
		.task_ch = 0,
		.polarity = NRF_GPIOTE_POLARITY_TOGGLE,
		.init_val = NRF_GPIOTE_INITIAL_VALUE_HIGH
	};

	printk("App core is starting.\n");

	// Initialize the selected timer
	timerConfiguration.bit_width = NRF_TIMER_BIT_WIDTH_32;
	ret = nrfx_timer_init(&timerInstance, &timerConfiguration, timerInterruptHandler);
	if (ret != NRFX_SUCCESS) {
		printk("Error : failed to initialize the timer (0x%08X).", ret);
		return -1;
	}
	nrfx_timer_clear(&timerInstance);

	ret = nrfx_gpiote_init(&gpioteInstance, NRFX_GPIOTE_DEFAULT_CONFIG_IRQ_PRIORITY);
	if ((ret != NRFX_SUCCESS) && (ret != NRFX_ERROR_ALREADY)) {
		printk("Error : failed to initialize the GPIOTE instance (0x%08X).", ret);
		return -1;
	}
	ret = nrfx_gpiote_output_configure(&gpioteInstance, GPIO_PIN, &gpioteOutputConfiguration, &gpioteTaskConfiguration);
	if (ret != NRFX_SUCCESS) {
		printk("Error : failed to configure the GPIOTE output (0x%08X).", ret);
		return -1;
	}
	nrfx_gpiote_out_task_enable(&gpioteInstance, GPIO_PIN);

	// Configure the interrupt firing frequency
	IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID)), DT_IRQ(DT_NODELABEL(TIMER_DT_LABEL), priority), timer_interrupt_handler, 0);
	ticks = nrfx_timer_us_to_ticks(&timerInstance, 2000000);
	nrfx_timer_extended_compare(&timerInstance, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

	configureDppi();

	// Enable the timer only when all DPPI paths are configured
	nrfx_timer_enable(&timerInstance);

	printk("App core is sleeping forever.\n");
	k_sleep(K_FOREVER);

	return 0;
}
