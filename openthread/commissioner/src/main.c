/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>

#include <zephyr/net/openthread.h>
#include <openthread/thread.h>

LOG_MODULE_REGISTER(ot_commissioner);

#define OT_CONNECTION_LED DK_LED1
#define OT_JOINER_PSKD "J01NU5"
#define OT_JOINER_TIMEOUT 2000

static void ot_commissioner_state_changed(otCommissionerState aState, void *aContext)
{
	const char *const kStateString[] = {
        "disabled",    // (0) OT_COMMISSIONER_STATE_DISABLED
        "petitioning", // (1) OT_COMMISSIONER_STATE_PETITION
        "active",      // (2) OT_COMMISSIONER_STATE_ACTIVE
    };
	LOG_INF("Commissioner state changed: %s", kStateString[aState]);
	
	if (aState == OT_COMMISSIONER_STATE_ACTIVE) {
		LOG_INF("Add joiner with PSKd %s", OT_JOINER_PSKD);
		otCommissionerAddJoiner(openthread_get_default_instance(), NULL, OT_JOINER_PSKD, OT_JOINER_TIMEOUT);
	}
}

static void on_thread_state_changed(otChangedFlags flags, struct openthread_context *ot_context,
				    void *user_data)
{
	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
			dk_set_led_on(OT_CONNECTION_LED);
			otCommissionerStart(ot_context->instance, &ot_commissioner_state_changed,
			NULL, NULL);
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

void ot_commissioner_init()
{
	openthread_state_changed_cb_register(openthread_get_default_context(), &ot_state_chaged_cb);
	openthread_start(openthread_get_default_context());
}

int main(void)
{
	LOG_INF("Start OpenThread commissioner example");
	int err;
	
	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}

	ot_commissioner_init();
}
