/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <zephyr/types.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/net/socket.h>
#include <dk_buttons_and_leds.h>

#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>

#include <zephyr/net/icmp.h>
#include "icmpv4.h"
#include "ping.h"

LOG_MODULE_REGISTER(ping);

const struct shell *shell_backend;
static struct net_icmp_ctx ctx;

struct k_work_delayable ping_work;
struct k_work_delayable ping_response_timeout;

#define PING_WORK_DELAY K_MSEC(50)
#define PING_RESPONSE_TIMEOUT K_MSEC(1.5*CONFIG_WIFI_TWT_INTERVAL_MS)

#define WAIT_TIME K_SECONDS(1)

static struct sockaddr_in addr4;
static int sequence = 0;
static int recv_seq = 0;

static int handle_ping_reply(struct net_icmp_ctx *ctx,
						   struct net_pkt *pkt,
					       struct net_icmp_ip_hdr *hdr,
					       struct net_icmp_hdr *icmp_hdr,
						   void *user_data)
{
	struct k_sem *sem_wait = user_data;
	struct net_ipv4_hdr *ip_hdr = hdr->ipv4;
	k_work_cancel_delayable(&ping_response_timeout);
	dk_set_led(DK_LED2, 0);

	NET_PKT_DATA_ACCESS_CONTIGUOUS_DEFINE(icmp_access,
					      struct net_icmpv4_echo_req);
	uint32_t cycles;
	struct net_icmpv4_echo_req *icmp_echo;
	char time_buf[16] = { 0 };

	icmp_echo = (struct net_icmpv4_echo_req *)net_pkt_get_data(pkt,
								&icmp_access);
	if (icmp_echo == NULL) {
		return -NET_DROP;
	}

	net_pkt_skip(pkt, sizeof(*icmp_echo));

	if (net_pkt_remaining_data(pkt) >= sizeof(uint32_t)) {
		if (net_pkt_read_be32(pkt, &cycles)) {
			return -NET_DROP;
		}

		cycles = k_cycle_get_32() - cycles;

		snprintf(time_buf, sizeof(time_buf),
#ifdef CONFIG_FPU
			 "time=%.2f ms",
			 (double)((uint32_t)k_cyc_to_ns_floor64(cycles) / 1000000.f)
#else
			 "time=%d ms",
			 ((uint32_t)k_cyc_to_ns_floor64(cycles) / 1000000)
#endif
			);
	}
	char ipv4_src_addr[NET_IPV4_ADDR_LEN];
	char ipv4_dst_addr[NET_IPV4_ADDR_LEN];
	inet_ntop(AF_INET, &ip_hdr->src, ipv4_src_addr, sizeof(ipv4_src_addr));
	inet_ntop(AF_INET, &ip_hdr->dst, ipv4_dst_addr, sizeof(ipv4_dst_addr));
	
	recv_seq = ntohs(icmp_echo->sequence);
	LOG_INF("%d bytes received from %s: seq=%d ttl=%d "
		 "%s",
		 ntohs(ip_hdr->len) - net_pkt_ipv6_ext_len(pkt) -
		 NET_ICMPH_LEN,
		 ipv4_src_addr,
		 recv_seq,
		 ip_hdr->ttl,
		 time_buf);

	net_pkt_unref(pkt);
	(void)net_icmp_cleanup_ctx(ctx);
	k_sem_give(sem_wait);
	return NET_OK;
} 

static void ping_response_timeout_handler()
{
	LOG_ERR("Ping timed out, clearing ICMP");
	(void)net_icmp_cleanup_ctx(&ctx);
	sequence = recv_seq;
	return;
}

static void ping_work_handler(struct k_work *item)
{
	if (recv_seq < sequence) {
		LOG_ERR("Ping already schedueled");
		return;
	}

	int err;
	struct net_icmp_ping_params params;
	struct net_if *iface = net_if_get_default();

	sequence++;

	err = net_icmp_init_ctx(&ctx, NET_ICMPV4_ECHO_REPLY, 0, handle_ping_reply);
	if (err) {
		LOG_ERR("Cannot init ICMP, err: %d, %s", err, strerror(err));
		goto fail;
	}
	
	addr4.sin_family = AF_INET;
	memcpy(&addr4.sin_addr, &iface->config.dhcpv4.server_id, sizeof(addr4.sin_addr));

	if (err) {
        LOG_ERR("Invalid address, err: %d, %s", errno, strerror(errno));
		goto fail;
    }
	
	params.identifier = 0;
	params.sequence = sequence;
	params.tc_tos = 0;
	params.priority = -1;
	params.data = NULL;
	params.data_size = 4;

	static struct k_sem sem_wait;
	k_sem_init(&sem_wait, 0, 1);
	
	err = net_icmp_send_echo_request(&ctx,
							net_if_get_default(),
							(struct sockaddr *)&addr4,
							&params,
							&sem_wait);
	
	if (err) {
        LOG_ERR("Failed to send ping: %d, %s", errno, strerror(errno));
		goto fail;
    }
	LOG_INF("Ping scheduled, seq=%d", sequence);
	err = k_sem_take(&sem_wait, K_MSEC(MSEC_PER_SEC));
	k_work_schedule(&ping_response_timeout, PING_RESPONSE_TIMEOUT);
	return;

fail:
	sequence = recv_seq;
	k_work_reschedule(&ping_work, PING_WORK_DELAY);
	return;
}

void ping_schedule()
{
    k_work_schedule(&ping_work, K_NO_WAIT);
}

void ping_init()
{
    shell_backend = shell_backend_uart_get_ptr();

    k_work_init_delayable(&ping_work, ping_work_handler);
	k_work_init_delayable(&ping_response_timeout, ping_response_timeout_handler);
}
