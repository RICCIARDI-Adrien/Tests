#include <nrfx_timer.h>
#include <zephyr/kernel.h>

#define TIMER_DT_LABEL timer022
#define TIMER_INSTANCE_ID 022

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

int main(void)
{
	nrfx_timer_t timerInstance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_ID);
	nrfx_timer_config_t timerConfiguration = NRFX_TIMER_DEFAULT_CONFIG(1000000);
	uint32_t ticks;
	nrfx_err_t ret;

	printk("Radio core is starting.\n");

	// Initialize the selected timer
	timerConfiguration.bit_width = NRF_TIMER_BIT_WIDTH_32;
	ret = nrfx_timer_init(&timerInstance, &timerConfiguration, timerHandler);
	if (ret != NRFX_SUCCESS) {
		printk("Error : failed to initialize the timer (0x%08X).", ret);
		return -1;
	}
	nrfx_timer_clear(&timerInstance);

	// Configure the timer channel 0 to overflow each second
	IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INSTANCE_ID)), DT_IRQ(DT_NODELABEL(TIMER_DT_LABEL), priority), timerInterruptHandler, 0);
	ticks = nrfx_timer_us_to_ticks(&timerInstance, 1000000);
	nrfx_timer_extended_compare(&timerInstance, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

	// TODO

	// Start ticking the SPI transfers
	nrfx_timer_enable(&timerInstance);

	printk("Radio core is sleeping forever.\n");
	k_sleep(K_FOREVER);
	return 0;
}
