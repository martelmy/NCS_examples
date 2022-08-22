<i>Latest nRF Connect version tested with: v2.0.2</i>

# Zigbee Multi Sensor

The Zigbee Multi Sensor demonstrates how the Temperature and Pressure sensor, similar to the [Zigbee Multi Sensor Example in nRF5 SDK for Thread & Zigbee](https://infocenter.nordicsemi.com/topic/sdk_tz_v4.1.0/zigbee_multi_sensor_example.html).
<br>

## Overview

The example declares the Temperature Measurement cluster and the Pressure Measurement cluster from the Zigbee Cluster Library. The sensor data is simulated using the [simulated sensor driver](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/drivers/sensor_sim.html), but this can be switched out for a real sensor.

The measured value attributes of the Temperature Measurement cluster and Pressure Measurement cluster are updated regularly with the simulated sensor data. To receive the attribute values, Zigbee devices can send a Read Attributes command, or they can configure attribute reporting with the Configure Reporting command. To test this example, use the [Zigbee Shell sample](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/samples/zigbee/shell/README.html) or configure the [Zigbee shell library](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/zigbee/shell.html#lib-zigbee-shell) in another Zigbee example:
```
CONFIG_ZIGBEE_SHELL=y
```

## Requirements
The sample supports the following development kits:

```
nrf52840dk_nrf52840
nrf52833dk_nrf52833
nrf5340dk_nrf5340
```
You will need an additional Zigbee device to read the attribute values.
<br>
<br>

## Building and running

Build the sample as described in [Building and programming an application](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_programming.html)
<br>
<br>

## Testing

The Zigbee shell commands in this part should be run on a device programmed with the [Zigbee Shell sample](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/samples/zigbee/shell/README.html) or one of the other Zigbee samples in nRF Connect SDK with the [Zigbee shell library](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/zigbee/shell.html#lib-zigbee-shell) configured. For how to connect and configure the shell device see [Testing an application](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_testing.html#gs-testing). 
<br>
<br>

1\. Set up the Zigbee shell device and start the network.

Set the shell device to work as a coordinator:
```
bdb role zc
```
You can also set the role as router with ```bdb role zr```, but then you must have another Zigbee device acting as coordinator.

Start a new Zigbee network if the device is a coordinator, or make the device start looking for a network if it is a router:
```
bdb start
```
2\. Turn on the multi sensor and wait for it to join the network.

3\. Find the multi sensor device.

Send a match descriptor request looking for a device with the two input clusters Temperature Measurement cluster (ID: ```0x0402```) and Pressure Measurement cluster (ID: ```0x0403```) from the Home Automation profile (ID: ```0x0104```).
```
zdo match_desc 0xfffd 0xfffd 0x0104 2 0x0402 0x0403 0
```
You should get a response containing the network address and endpoint of the device:
```
Sending broadcast request
src_addr=634b ep=12
```

4\. Find the long address of the sensor.

Now that you have the network address you can use this to send an IEEE address request to get the long address of the multi sensor.
```
zdo ieee_addr 0x634b
```
The output is the IEEE address of the sensor:
```
0B010E0405060708
```

5\. Get your own IEEE address.
```
zdo eui64
```
This command will return the IEEE address of the shell device issuing the command
```
0B010E4050607080
```
6\. Bind the remote endpoint (on the multi sensor) to the shell device for the Temperature Measurement cluster and Pressure Measurement cluster.
```
zdo bind on 0B010E0405060708 12 0B010E4050607080 64 0x0402 0x634b
zdo bind on 0B010E0405060708 12 0B010E4050607080 64 0x0403 0x634b
```
Here it is important to make sure that you are using the correct addresses. The format of the zdo bind on command is as follows:
<pre><code>zdo bind on <i>h:source_eui64 d:source_ep h:dst_addr d:dst_ep h:source_cluster_id h:request_dst_addr</i></pre></code>
The first address <i>source_eui64</i> is the IEEE address of the multi sensor, the one you found in step 4. The second address <i>dst_addr</i> is the IEEE address of the shell device, which was found in step 5. The shorter address at the end, <i>request_dst_addr</i> is the network address of the multi sensor found with the match_desc in step 3.

You might start receiving attribute reports from the multi sensor already after creating the binding:
```
<inf> zigbee_shell_report: Received value updates from the remote node 0x634b
<inf> zigbee_shell_report: Profile: 0x0104 Cluster: 0x0402 Attribute: 0x0000 Type: 41 Value: 21
```
If you are still not receiving attribute reports from the multi sensor, you can try the following:

Subscribe on the attribute changes with the zcl subscribe command:
```
zcl subscribe on 0x634b 12 0x0402 0x0104 0x00 41
zcl subscribe on 0x634b 12 0x0403 0x0104 0x00 41
```
Enable logs from the zigbee_shell_report library:
```
log enable inf zigbee_shell_report
```

For a full list of Zigbee shell commands see the [Zigbee shell library documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/zigbee/shell.html).
