
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/wifi_mgmt.h>
#include "ca_certificate.h"
#include "network.h"

#define WIFI_SOCKET_MGMT_EVENTS (NET_EVENT_WIFI_CONNECT_RESULT | \
		    NET_EVENT_WIFI_DISCONNECT_RESULT)

#define PEER_PORT 4242

#if !defined(CONFIG_NET_CONFIG_PEER_IPV4_ADDR)
#define CONFIG_NET_CONFIG_PEER_IPV4_ADDR "192.168.32.101"
#endif

#if !defined(CONFIG_NET_CONFIG_PEER_IPV6_ADDR)
#define CONFIG_NET_CONFIG_PEER_IPV6_ADDR ""
#endif

K_SEM_DEFINE(wifi_connected, 0, 1);
K_SEM_DEFINE(ipv4_obtained, 0, 1);

static bool wifi_is_connected;
static bool ip_obtained;

static struct net_mgmt_event_callback mgmt_cb;
static struct net_mgmt_event_callback net_mgmt_ipv4_callback;

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
			  uint32_t mgmt_event, struct net_if *iface)
{

    const struct wifi_status *wifi_status = (const struct wifi_status *)cb->info;

	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
		if (wifi_status->status) {
			LOG_INF("Connection attempt failed, status code: %d", wifi_status->status);
			return;
		}
        LOG_INF("Wi-Fi Connected, waiting for IP address");
        wifi_is_connected = true;
		k_sem_give(&wifi_connected);
		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		LOG_INF("Disconnected");
        wifi_is_connected = false;
		break;
	default:
        LOG_ERR("Unknown event: %d", mgmt_event);
		break;
	}
}

static void ipv4_mgmt_event_handler(struct net_mgmt_event_callback *cb,
				    uint32_t event, struct net_if *iface)
{
	switch (event) {
	case NET_EVENT_IPV4_ADDR_ADD:
		LOG_INF("IPv4 address acquired");
		ip_obtained = true;
		k_sem_give(&ipv4_obtained);
		break;
	case NET_EVENT_IPV4_ADDR_DEL:
		LOG_INF("IPv4 address lost");
		ip_obtained = false;
		break;
	default:
		LOG_DBG("Unknown event: 0x%08X", event);
		return;
	}
}

static void init_app()
{    
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
	int err;
    // Add CA certificate
    err = tls_credential_add(CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE,
                         ca_certificate, sizeof(ca_certificate));
    if (err < 0) {
		LOG_ERR("Failed to register public certificate: %d", err);
	}
#if defined(CONFIG_MBEDTLS_KEY_EXCHANGE_PSK_ENABLED)
	err = tls_credential_add(PSK_TAG,
				TLS_CREDENTIAL_PSK,
				psk,
				sizeof(psk));
	if (err < 0) {
		LOG_ERR("Failed to register PSK: %d", err);
	}
	err = tls_credential_add(PSK_TAG,
				TLS_CREDENTIAL_PSK_ID,
				psk_id,
				sizeof(psk_id) - 1);
	if (err < 0) {
		LOG_ERR("Failed to register PSK ID: %d", err);
	}
#endif //CONFIG_MBEDTLS_KEY_EXCHANGE_PSK_ENABLED

#endif //CONFIG_NET_SOCKETS_SOCKOPT_TLS

    if (IS_ENABLED(CONFIG_NET_CONNECTION_MANAGER)) {
		LOG_INF("Registering wifi events");
		net_mgmt_init_event_callback(&mgmt_cb,
					     wifi_mgmt_event_handler, WIFI_SOCKET_MGMT_EVENTS);
		net_mgmt_add_event_callback(&mgmt_cb);
		LOG_INF("Registering IPv4 events");
		net_mgmt_init_event_callback(&net_mgmt_ipv4_callback, ipv4_mgmt_event_handler,
				     NET_EVENT_IPV4_ADDR_ADD | NET_EVENT_IPV4_ADDR_DEL);
		net_mgmt_add_event_callback(&net_mgmt_ipv4_callback);

	}

}

static int socket_setup(int *sock, struct sockaddr *addr, socklen_t addrlen) {
    int ret = 0;
    
    // Create socket
	errno = 0;
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
    *sock = socket(addr->sa_family, SOCK_STREAM, IPPROTO_TLS_1_2);
#else
    *sock = socket(addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
#endif
    if (*sock < 0)
    {
        LOG_ERR("Failed to create TCP socket, errno: %d", errno);
        return errno;
    }

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
    // Configure socket options
    sec_tag_t sec_tag_opt[] = {
        CA_CERTIFICATE_TAG,
#if defined(CONFIG_MBEDTLS_KEY_EXCHANGE_PSK_ENABLED)
		PSK_TAG,
#endif //CONFIG_MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
    };
    ret = setsockopt(*sock, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_opt, sizeof(sec_tag_opt));
    if (ret < 0) {
        LOG_ERR("Failed to set TLS_SEC_TAG_LIST option, errno: %d", errno);
    }

    ret = setsockopt(*sock, SOL_TLS, TLS_HOSTNAME, TLS_PEER_HOSTNAME, sizeof(TLS_PEER_HOSTNAME));
    if (ret < 0) {
        LOG_ERR("Failed to set TLS_HOSTNAME option, errno: %d", errno);
    }
#endif //CONFIG_NET_SOCKETS_SOCKOPT_TLS

    ret = connect(*sock, addr, addrlen);
    if (ret < 0) {
		LOG_ERR("Cannot connect to TCP remote, errno: %d", errno);
	}

    return ret;
}


int start_tcp(int *sock)
{
	int ret = 0;
	struct sockaddr_in addr4;
	struct sockaddr_in6 addr6;

	if (IS_ENABLED(CONFIG_NET_IPV6)) {
		addr6.sin6_family = AF_INET6;
		addr6.sin6_port = htons(PEER_PORT);
		inet_pton(AF_INET6, CONFIG_NET_CONFIG_PEER_IPV6_ADDR,
			  &addr6.sin6_addr);

		ret = socket_setup(sock,(struct sockaddr *)&addr6, sizeof(addr6));
		if (ret < 0) {
			return ret;
		}
	}

	if (IS_ENABLED(CONFIG_NET_IPV4)) {
		addr4.sin_family = AF_INET;
		addr4.sin_port = htons(PEER_PORT);
		inet_pton(AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR,
			  &addr4.sin_addr);

		ret = socket_setup(sock, (struct sockaddr *)&addr4, sizeof(addr4));
		if (ret < 0) {
			return ret;
		}
	}
    return ret;
}


void send_tcp_data(int *sock)
{
    const char *content[] = {
		"foobar",
		"chunked",
		"last"
	};
	char tmp[64];
	int i, pos = 0;
    for (i = 0; i < ARRAY_SIZE(content); i++) {
		pos += snprintk(tmp + pos, sizeof(tmp) - pos,
				"%x\r\n%s\r\n",
				(unsigned int)strlen(content[i]),
				content[i]);
	}
    pos += snprintk(tmp + pos, sizeof(tmp) - pos, "0\r\n\r\n");
	errno = 0;
    int ret = send(*sock, tmp, pos, 0);
	printk("send() return %d\n", ret);
	if (ret == -1) {
		LOG_ERR("Error sending data to peer, errno: %d", errno);
	}
}

static void run_tcp(int *sock)
{
    // int ret;
    // char buf[RECV_BUF_SIZE];
	send_tcp_data(sock);
}


void main(void)
{
    int err = 0;
    int sock = -1;
    printk("Wi-Fi Socket Echo Client started, sleep 1 second for IF to awake\n");
	k_sleep(K_SECONDS(1));
    init_app();
    wifi_connect();
	LOG_INF("Wait for Wi-fi connection");
	k_sem_take(&wifi_connected, K_FOREVER);
	LOG_INF("Wait for DHCP");
	k_sem_take(&ipv4_obtained, K_FOREVER);
    err = start_tcp(&sock);

    while (ip_obtained && wifi_is_connected && (err == 0)) {        
        run_tcp(&sock);
        k_sleep(K_SECONDS(5));
    }
	LOG_INF("Link went down, exiting");
}