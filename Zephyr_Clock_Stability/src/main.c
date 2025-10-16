#include <nrfx_gpiote.h>
#include <nrfx_timer.h>
#include <zephyr/kernel.h>

#define TIMER_DT_LABEL timer132
#define TIMER_INSTANCE_ID 132
#define GPIOTE_INSTANCE_ID 130

#define GPIO_PIN (9 << 5)

static nrfx_gpiote_t gpiote_instance = NRFX_GPIOTE_INSTANCE(GPIOTE_INSTANCE_ID);

ISR_DIRECT_DECLARE(timer_interrupt_handler)
{
    // Invoke the handler at nRFx driver level, which in turn will invoke the event handler registered below
    NRFX_TIMER_INST_HANDLER_GET(TIMER_INSTANCE_ID)();
    ISR_DIRECT_PM();
    return 1;
}

static void timer_handler(nrf_timer_event_t event_type, void *p_context)
{
	printk("CIAO\n");

	// Manually toggle the GPIO
	nrfx_gpiote_out_task_trigger(&gpiote_instance, GPIO_PIN);
}

int main(void)
{
	int ret;
	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	nrfx_timer_t timer_instance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_ID);
	uint32_t ticks;
	nrfx_gpiote_output_config_t gpiote_output_config =
	{
		.drive = NRF_GPIO_PIN_S0S1,
		.input_connect = NRF_GPIO_PIN_INPUT_DISCONNECT,
		.pull = NRF_GPIO_PIN_NOPULL
	};
	nrfx_gpiote_task_config_t gpiote_task_config =
	{
		.task_ch = 0,
		.polarity = NRF_GPIOTE_POLARITY_TOGGLE,
		.init_val = NRF_GPIOTE_INITIAL_VALUE_HIGH
	};

	printk("App core is starting.\n");

	// Initialize the selected timer
	timer_config.bit_width = NRF_TIMER_BIT_WIDTH_32;
	ret = nrfx_timer_init(&timer_instance, &timer_config, timer_handler);
	if (ret != NRFX_SUCCESS) {
		printk("Error : failed to initialize the timer (0x%08X).", ret);
		return -1;
	}
	nrfx_timer_clear(&timer_instance);

	ret = nrfx_gpiote_init(&gpiote_instance, NRFX_GPIOTE_DEFAULT_CONFIG_IRQ_PRIORITY);
	if ((ret != NRFX_SUCCESS) && (ret != NRFX_ERROR_ALREADY)) {
		printk("Error : failed to initialize the GPIOTE instance (0x%08X).", ret);
		return -1;
	}
	ret = nrfx_gpiote_output_configure(&gpiote_instance, GPIO_PIN, &gpiote_output_config, &gpiote_task_config);
	if (ret != NRFX_SUCCESS) {
		printk("Error : failed to configure the GPIOTE output (0x%08X).", ret);
		return -1;
	}
	nrfx_gpiote_out_task_enable(&gpiote_instance, GPIO_PIN);

	// Configure the interrupt firing frequency
	IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID)), DT_IRQ(DT_NODELABEL(TIMER_DT_LABEL), priority), timer_interrupt_handler, 0);
	ticks = nrfx_timer_us_to_ticks(&timer_instance, 2000000);
	nrfx_timer_extended_compare(&timer_instance, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
	nrfx_timer_enable(&timer_instance);

	printk("App core is sleeping forever.\n");
	k_sleep(K_FOREVER);

	return 0;
}
