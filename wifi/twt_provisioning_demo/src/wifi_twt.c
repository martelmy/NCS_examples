/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>

#include "wifi_twt.h"

LOG_MODULE_REGISTER(wifi_twt);

#define TWT_MGMT_EVENTS (NET_EVENT_WIFI_TWT | NET_EVENT_WIFI_TWT_SLEEP_STATE)

#define TWT_WAKE_INTERVAL_MS 65
#define TWT_INTERVAL_MS	     15000

bool nrf_wifi_twt_enabled = 0;
uint8_t twt_flow_id = 1;

static struct net_mgmt_event_callback twt_mgmt_cb;

static void handle_wifi_twt_event(struct net_mgmt_event_callback *cb)
{
	const struct wifi_twt_params *resp = (const struct wifi_twt_params *)cb->info;

	if (resp->operation == WIFI_TWT_TEARDOWN) {
		LOG_INF("TWT teardown received for flow ID %d\n", resp->flow_id);
		nrf_wifi_twt_enabled = 0;
		return;
	}

	twt_flow_id = resp->flow_id;

	/* Check if a TWT response was received or timed out */
	if (resp->resp_status == WIFI_TWT_RESP_RECEIVED) {
		LOG_INF("TWT response: %s", wifi_twt_setup_cmd_txt(resp->setup_cmd));
	} else {
		LOG_INF("TWT response timed out\n");
		return;
	}
	/* Check if TWT setup was accepted and print parameters */
	if (resp->setup_cmd == WIFI_TWT_SETUP_CMD_ACCEPT) {
		nrf_wifi_twt_enabled = 1;

		LOG_INF("== TWT negotiated parameters ==");
		LOG_INF("TWT Dialog token: %d", resp->dialog_token);
		LOG_INF("TWT flow ID: %d", resp->flow_id);
		LOG_INF("TWT negotiation type: %s",
			wifi_twt_negotiation_type_txt(resp->negotiation_type));
		LOG_INF("TWT responder: %s", resp->setup.responder ? "true" : "false");
		LOG_INF("TWT implicit: %s", resp->setup.implicit ? "true" : "false");
		LOG_INF("TWT announce: %s", resp->setup.announce ? "true" : "false");
		LOG_INF("TWT trigger: %s", resp->setup.trigger ? "true" : "false");
		LOG_INF("TWT wake interval: %d ms (%d us)",
			resp->setup.twt_wake_interval / USEC_PER_MSEC,
			resp->setup.twt_wake_interval);
		LOG_INF("TWT interval: %lld s (%lld us)", resp->setup.twt_interval / USEC_PER_SEC,
			resp->setup.twt_interval);
		LOG_INF("===============================");
	}
}

static void twt_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
				   struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_WIFI_TWT:
		handle_wifi_twt_event(cb);
		break;
	case NET_EVENT_WIFI_TWT_SLEEP_STATE:
		int *twt_state;
		twt_state = (int *)(cb->info);
		LOG_INF("TWT sleep state: %s", *twt_state ? "awake" : "sleeping");
		break;
	}
}

int wifi_set_twt()
{
	struct net_if *iface = net_if_get_first_wifi();

	struct wifi_twt_params params = {0};

	params.negotiation_type = WIFI_TWT_INDIVIDUAL;
	params.flow_id = twt_flow_id;
	params.dialog_token = 1;

	if (!nrf_wifi_twt_enabled) {
		params.operation = WIFI_TWT_SETUP;
		params.setup_cmd = WIFI_TWT_SETUP_CMD_REQUEST;
		params.setup.responder = 0;
		params.setup.trigger = 0;
		params.setup.implicit = 1;
		params.setup.announce = 0;
		params.setup.twt_wake_interval = TWT_WAKE_INTERVAL_MS * USEC_PER_MSEC;
		params.setup.twt_interval = TWT_INTERVAL_MS * USEC_PER_MSEC;
	} else {
		params.operation = WIFI_TWT_TEARDOWN;
		params.setup_cmd = WIFI_TWT_TEARDOWN;
		twt_flow_id = twt_flow_id < WIFI_MAX_TWT_FLOWS ? twt_flow_id + 1 : 1;
		nrf_wifi_twt_enabled = 0;
	}

	if (net_mgmt(NET_REQUEST_WIFI_TWT, iface, &params, sizeof(params))) {
		LOG_ERR("%s with %s failed, reason : %s", wifi_twt_operation_txt(params.operation),
			wifi_twt_negotiation_type_txt(params.negotiation_type),
			wifi_twt_get_err_code_str(params.fail_reason));
		return -1;
	}
	LOG_INF("-------------------------------");
	LOG_INF("TWT operation %s requested", wifi_twt_operation_txt(params.operation));
	LOG_INF("-------------------------------");
	return 0;
}

void wifi_twt_init(void)
{
	net_mgmt_init_event_callback(&twt_mgmt_cb, twt_mgmt_event_handler, TWT_MGMT_EVENTS);
	net_mgmt_add_event_callback(&twt_mgmt_cb);
}