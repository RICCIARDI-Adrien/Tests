#include <helpers/nrfx_gppi.h>
#include <nrfx_gpiote.h>
#include <nrfx_timer.h>
#include <zephyr/kernel.h>

#define TIMER_DT_LABEL timer132
#define TIMER_INSTANCE_ID 132
#define GPIOTE_INSTANCE_ID 130

// Port 9 pin 0
#define GPIO_PIN (9 << 5)

static nrfx_timer_t timerInstance = NRFX_TIMER_INSTANCE(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID));
static nrfx_gpiote_t gpioteInstance = NRFX_GPIOTE_INSTANCE(NRF_GPIOTE_INST_GET(GPIOTE_INSTANCE_ID));

static void timerInterruptHandler(nrf_timer_event_t event_type, void *p_context)
{
	static uint32_t counter = 0;

	printk("CIAO %u.\n", counter);
	counter++;
}

static void configureDppi(void)
{
	uint32_t sourceEndpoint, destinationEndpoint;
	nrfx_gppi_handle_t gppiHandle;
	int ret;

	sourceEndpoint = nrf_timer_event_address_get(timerInstance.p_reg, NRF_TIMER_EVENT_COMPARE0);
	destinationEndpoint = nrf_gpiote_task_address_get(gpioteInstance.p_reg, NRF_GPIOTE_TASK_OUT_0);

	ret = nrfx_gppi_conn_alloc(sourceEndpoint, destinationEndpoint, &gppiHandle);
	if (ret != 0)
	{
		printk("Error : failed to allocated a GPPI connection (0x%08X).\n", ret);
		return;
	}

	nrfx_gppi_conn_enable(gppiHandle);
}

int main(void)
{
	int ret;
	nrfx_timer_config_t timerConfiguration = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	uint32_t ticks;
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
	if (ret != 0) {
		printk("Error : failed to initialize the timer (0x%08X).", ret);
		return -1;
	}
	nrfx_timer_clear(&timerInstance);

	ret = nrfx_gpiote_init(&gpioteInstance, NRFX_GPIOTE_DEFAULT_CONFIG_IRQ_PRIORITY);
	if ((ret != 0) && (ret != -EALREADY)) {
		printk("Error : failed to initialize the GPIOTE instance (0x%08X).", ret);
		return -1;
	}
	ret = nrfx_gpiote_output_configure(&gpioteInstance, GPIO_PIN, &gpioteOutputConfiguration, &gpioteTaskConfiguration);
	if (ret != 0) {
		printk("Error : failed to configure the GPIOTE output (0x%08X).", ret);
		return -1;
	}
	nrfx_gpiote_out_task_enable(&gpioteInstance, GPIO_PIN);

	// Configure the interrupt firing frequency
	IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID)), DT_IRQ(DT_NODELABEL(TIMER_DT_LABEL), priority), nrfx_timer_irq_handler, &timerInstance, 0);
	ticks = nrfx_timer_us_to_ticks(&timerInstance, 500000);
	nrfx_timer_extended_compare(&timerInstance, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

	configureDppi();

	// Enable the timer only when all DPPI paths are configured
	nrfx_timer_enable(&timerInstance);

	printk("App core is sleeping forever.\n");
	k_sleep(K_FOREVER);

	return 0;
}
