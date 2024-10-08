/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app);

int main(void)
{
	usb_enable(NULL);
	while(true){
		LOG_INF("Hello World");
		/* Wait 100ms for the host to do all settings */
		k_msleep(100);
	}
	return 0;
}
