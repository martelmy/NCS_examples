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

#include "icmpv4.h"
#include "ping.h"

LOG_MODULE_REGISTER(ping);

const struct shell *shell_backend;

struct k_work_delayable ping_work;
struct k_work_delayable ping_timeout_work;
#define PING_WORK_DELAY K_MSEC(50)
#define PING_TIMEOUT K_SECONDS(5)

#define WAIT_TIME K_SECONDS(1)

#define HOST_ADDR "8.8.8.8"
static struct sockaddr_in addr4;
static int sequence = 0;

static enum net_verdict handle_ping_reply(struct net_pkt *pkt,
					       struct net_ipv4_hdr *ip_hdr,
					       struct net_icmp_hdr *icmp_hdr);

static struct net_icmpv4_handler ping_handler = {
	.type = NET_ICMPV4_ECHO_REPLY,
	.code = 0,
	.handler = handle_ping_reply,
};

static enum net_verdict handle_ping_reply(struct net_pkt *pkt,
					       struct net_ipv4_hdr *ip_hdr,
					       struct net_icmp_hdr *icmp_hdr)
{
	k_work_cancel_delayable(&ping_timeout_work);
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
	

	LOG_INF("%d bytes from %s to %s: seq=%d ttl=%d "
		 "%s\n",
		 ntohs(ip_hdr->len) - net_pkt_ipv6_ext_len(pkt) -
		 NET_ICMPH_LEN,
		 ipv4_src_addr,
		 ipv4_dst_addr,
		 ntohs(icmp_echo->sequence),
		 ip_hdr->ttl,
		 time_buf);

	net_pkt_unref(pkt);
	return NET_OK;
} 

static void ping_timeout_work_handler()
{
    net_icmpv4_register_handler(&ping_handler);
}

static void ping_work_handler(struct k_work *item)
{
	int err;

	addr4.sin_family = AF_INET;
	err = net_addr_pton(AF_INET, HOST_ADDR, &addr4.sin_addr);

	if (err) {
        LOG_ERR("Invalid address, err: %d, %s", errno, strerror(errno));
		goto fail;
    }
	sequence++;
	err = net_icmpv4_send_echo_request(net_if_get_default(), 
							&addr4.sin_addr,
							0, sequence,
							0, -1, NULL, 4);
	
	if (err) {
        LOG_ERR("Failed to send ping: %d, %s", errno, strerror(errno));
		goto fail;
    }
	LOG_INF("Ping scheduled, seq=%d", sequence);
	k_work_schedule(&ping_timeout_work, PING_TIMEOUT);

	return;

fail:
	k_work_reschedule(&ping_work, PING_WORK_DELAY);
}

void ping_schedule()
{
    k_work_schedule(&ping_work, K_NO_WAIT);
}

void ping_init()
{
    shell_backend = shell_backend_uart_get_ptr();

    k_work_init_delayable(&ping_work, ping_work_handler);
	k_work_init_delayable(&ping_timeout_work, ping_timeout_work_handler);

	net_icmpv4_init();
	net_icmpv4_register_handler(&ping_handler);
}