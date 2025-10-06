#include <nrfx_spim.h>
#include <zephyr/kernel.h>

#define SPIM_INST_IDX 131

static void spim_handler(nrfx_spim_evt_t const * p_event, void * p_context)
{
    printk("INT\r\n");
}

int main(void)
{
	nrfx_spim_config_t spim_config =
	{
		.ss_active_high = false,
		.irq_priority = NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY,
		.frequency = MHZ(8),
		.mode = NRF_SPIM_MODE_0,
		.bit_order = NRF_SPIM_BIT_ORDER_MSB_FIRST,
		.miso_pull = NRF_GPIO_PIN_NOPULL,
		.skip_gpio_cfg = true,
		.skip_psel_cfg = true
	};
	nrfx_spim_t spim_instance = NRFX_SPIM_INSTANCE(SPIM_INST_IDX);
	nrfx_err_t ret;

	printk("App core is starting.\n");

	IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_SPIM_INST_GET(SPIM_INST_IDX)), IRQ_PRIO_LOWEST,
		NRFX_SPIM_INST_HANDLER_GET(SPIM_INST_IDX), 0, 0);

	ret = nrfx_spim_init(&spim_instance, &spim_config, spim_handler, NULL);
    if (ret != NRFX_SUCCESS) {
		printk("Error : failed to initialize the SPIM peripheral.\n");
		return -1;
	}

	return 0;
}
