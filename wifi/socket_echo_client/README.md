# Wi-Fi Socket Echo Client

<i> Based on nRF Connect SDK v2.3.0</i>

## Overview
This example demonstrates how to create and use TCP sockets over Wi-Fi. It sends a packet of dummy data to the server every 30 seconds.

## Requirements
- nRF7002 DK (nrf7002dk_nrf5340_cpuapp)
- Linux Host (for echo-server)
- [Zephyr Networking Tools](https://github.com/zephyrproject-rtos/net-tools.git)
<br>

## Configuration

### Configuration files
Replace the default network SSID and password in prj.conf with the SSID and password the device should connect to before building:
```
CONFIG_SOCKET_SAMPLE_SSID="MySSID"
CONFIG_SOCKET_SAMPLE_PASSWORD="MyPassword"
```

### TLS
<i>TLS support has not been tested </i>

This example has support for TLS. The certificate and private key can be found in [src](src). To enable TLS, uncomment the following configs in prj.conf:
```
CONFIG_NET_SOCKETS_SOCKOPT_TLS=y
CONFIG_POSIX_MAX_FDS=9
```

## Socket server
There are several ways to run the socket server. Here I show two options; netcat and Zephyr's Socket Echo Server.

### Netcat
Run netcat command to start the server.
```
nc -k -l -p 4242
```

### 

Download the Zephyr Networking Tools.
```
$ git clone https://github.com/zephyrproject-rtos/net-tools
$ cd net-tools
$ make
```

Run the echo-server. Make sure to replace ```enp0s25``` with your network interface.
```
$ ./echo-server -i enp0s25
```


## Building and running

Build the sample as described in [Building and programming an application](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_programming.html)

## Testing

After building the sample and programming it to your development kit, complete the following steps to test it:
1. Start the echo-server as explained in [Socket Echo server](#socket-echo-server):
```
$ echo-server -i enp0s25
```
2. Turn on the development kit.
3. Connect to a terminal emulator (e.g. using our [nRF Connect for VS Code extension](https://nrfconnect.github.io/vscode-nrf-connect/get_started/quick_debug.html#how-to-connect-to-the-terminal) or [PuTTY](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/testing.html#how-to-connect-with-putty)).
3. The device will connect to the server and start sending dummy data. The log should look similar to this:
 ```
*** Booting Zephyr OS build v3.2.99-ncs2 ***
Wi-Fi Socket Echo Client started, sleep 1 second for IF to awake
[00:00:01.661,529] <inf> app: Registering wifi events
[00:00:01.661,560] <inf> app: Registering IPv4 events
[00:00:02.575,866] <inf> network: Connection requested
[00:00:02.575,897] <inf> app: Wait for Wi-fi connection
[00:00:06.901,000] <inf> wifi_nrf: wifi_nrf_wpa_supp_authenticate:Authentication request sent successfully

[00:00:07.342,681] <inf> wifi_nrf: wifi_nrf_wpa_supp_associate: Association request sent successfully

[00:00:07.378,326] <inf> app: Wi-Fi Connected, waiting for IP address
[00:00:07.378,509] <inf> app: Wait for DHCP
[00:00:07.844,696] <inf> app: IPv4 address acquired
send() return 37
send() return 37
send() return 37
send() return 37
 ```

## Dependencies

This example uses the following Zephyr libraries:
- [BSD Sockets](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/connectivity/networking/api/sockets.html)
- [Network Management](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/connectivity/networking/api/net_mgmt.html)
- [Network Interface](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/connectivity/networking/api/net_if.html)

Documentation and error codes for the socket functions can be found here: 
- [socket()](https://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html)
- [setsockopt()](https://pubs.opengroup.org/onlinepubs/9699919799/functions/setsockopt.html)
- [connect()](https://pubs.opengroup.org/onlinepubs/9699919799/functions/connect.html)
- [send()](https://pubs.opengroup.org/onlinepubs/9699919799/functions/send.html)

To find the error corresponding to the errno number, see [zephyr/errno.h](https://github.com/zephyrproject-rtos/zephyr/blob/main/lib/libc/minimal/include/errno.h). 