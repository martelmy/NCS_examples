/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <version.h>

#include <zephyr/net/openthread.h>
#include <openthread/thread.h>

LOG_MODULE_REGISTER(ot_joiner);

#define OT_CONNECTION_LED DK_LED1
#define OT_JOINER_PSKD "J01NU5"

#define WAIT_TIME_FOR_OT_CON K_SECONDS(10)
static K_SEM_DEFINE(connected_sem, 0, 1);

static void on_thread_state_changed(otChangedFlags flags, struct openthread_context *ot_context,
				    void *user_data)
{
	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
			dk_set_led_on(OT_CONNECTION_LED);
			LOG_INF("Connected to Thread network");
			break;
		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
		default:
			dk_set_led_off(OT_CONNECTION_LED);
			break;
		}
	}
}

static struct openthread_state_changed_cb ot_state_chaged_cb = {
	.state_changed_cb = on_thread_state_changed
};

/* call back Thread device joiner */
static void ot_joiner_start_handler(otError error, void *context)
{
	switch (error) {

	case OT_ERROR_NONE:
		LOG_INF("Join success");
		k_sem_give(&connected_sem);
	break;

	default:
		LOG_ERR("Join failed [%s]", otThreadErrorToString(error));
	break;
	}
}

void ot_start_joiner()
{
	LOG_INF("Starting joiner");

	otInstance *instance = openthread_get_default_instance();
	struct openthread_context *context = openthread_get_default_context();

	openthread_api_mutex_lock(context);

	otIp6SetEnabled(instance, true);
	otJoinerStart(instance, OT_JOINER_PSKD, NULL,
				"Zephyr", "Zephyr",
				KERNEL_VERSION_STRING, NULL,
				&ot_joiner_start_handler, NULL);

	openthread_api_mutex_unlock(context);
}

void ot_joiner_init()
{
	otError err = 0;

	openthread_state_changed_cb_register(openthread_get_default_context(), &ot_state_chaged_cb);

	ot_start_joiner();
	err = k_sem_take(&connected_sem, WAIT_TIME_FOR_OT_CON);
	struct openthread_context *context = openthread_get_default_context();

	LOG_INF("Starting OpenThread");
	openthread_api_mutex_lock(context);

	err = otThreadSetEnabled(openthread_get_default_instance(), true);
	if (err != OT_ERROR_NONE) {
		LOG_ERR("Starting openthread: %d (%s)", err, otThreadErrorToString(err));
	}

	openthread_api_mutex_unlock(context);
}

int main(void)
{
	LOG_INF("Start OpenThread joiner example");
	int err;

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}

	ot_joiner_init();
}
