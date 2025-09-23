#include <nrfx_timer.h>
#include <zephyr/kernel.h>

#define TIMER_DT_LABEL timer132
#define TIMER_INSTANCE_ID 132

// Autonomous Alarm Timer ISR
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
}

int main(void)
{
	int ret;
	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	nrfx_timer_t timer_instance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_ID);
	uint32_t ticks;

	printk("App core is starting.\n");

	// Initialize the selected timer
	timer_config.bit_width = NRF_TIMER_BIT_WIDTH_32;
	ret = nrfx_timer_init(&timer_instance, &timer_config, timer_handler);
	if (ret != NRFX_SUCCESS) {
		printk("Error : failed to initialize the timer (%d).", ret);
		return -1;
	}
	nrfx_timer_clear(&timer_instance);

	// Configure the interrupt firing frequency
	IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID)), DT_IRQ(DT_NODELABEL(TIMER_DT_LABEL), priority), timer_interrupt_handler, 0);
	ticks = nrfx_timer_us_to_ticks(&timer_instance, 2000000);
	nrfx_timer_extended_compare(&timer_instance, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
	nrfx_timer_enable(&timer_instance);

	printk("App core is sleeping forever.\n");
	k_sleep(K_FOREVER);

	return 0;
}
