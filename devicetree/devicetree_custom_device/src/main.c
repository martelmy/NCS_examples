/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

#define CUSTOM_DEVICE DT_NODELABEL(my_custom_device)
static const struct gpio_dt_spec input_gpio = GPIO_DT_SPEC_GET(CUSTOM_DEVICE, in_gpios);
static const struct gpio_dt_spec output_gpio = GPIO_DT_SPEC_GET(CUSTOM_DEVICE, out_gpios);

void main(void)
{
	int ret;

	printk("Starting custom devicetree device example\n");

	if (!device_is_ready(input_gpio.port)) {
		printk("Input GPIOs not ready\n");
		return;
	}
	if (!device_is_ready(output_gpio.port)) {
		printk("Output GPIOs not ready\n");
		return;
	}

	ret = gpio_pin_configure_dt(&output_gpio, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}
	ret = gpio_pin_configure_dt(&input_gpio, GPIO_INPUT);
	if (ret < 0) {
		return;
	}

	int gpio_value;

	while (1) {
		printk("----------------------------------\n");
		gpio_value = gpio_pin_get_dt(&input_gpio);
		printk("GPIO value: %d\n", gpio_value);
		k_msleep(SLEEP_TIME_MS);
		printk("Toggling output GPIO\n");
		ret = gpio_pin_toggle_dt(&output_gpio);
		if (ret < 0) {
			return;
		}
		k_msleep(SLEEP_TIME_MS);
		gpio_value = gpio_pin_get_dt(&input_gpio);
		printk("GPIO value: %d\n", gpio_value);
		k_msleep(SLEEP_TIME_MS*5);
	}
}
