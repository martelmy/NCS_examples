#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define LONG_PRESS_TIMEOUT K_SECONDS(1)
struct k_timer long_press_timer;

static void button_handler(uint32_t button_state, uint32_t has_changed)
{
if (DK_BTN1_MSK & has_changed) {
		if (DK_BTN1_MSK & button_state) {
			/* Button changed its state to pressed */
			k_timer_start(&long_press_timer, LONG_PRESS_TIMEOUT, K_NO_WAIT);
		} else {
            /* Button changed its state to released */
			if (k_timer_status_get(&long_press_timer) > 0) {
				/* Timer expired before button was released, indicates long press */
				LOG_INF("Long press");
			} else {
				LOG_INF("Short press");
				k_timer_stop(&long_press_timer);
			}
		}
	}
}

int main(void)
{
    LOG_INF("Long press button example started");

    if (dk_buttons_init(button_handler) != 0) {
        LOG_ERR("Failed to initialize the buttons library");
	}

    return 0;
}
