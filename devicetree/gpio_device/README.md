# GPIO Device

The GPIO Device demonstrates how to use a nRF52840 DK to control buttons and view LEDs on other devices.
It uses GPIOs to 'press' the button on another device, and to read the state of a LED on a device. Zephyr shell is used to control what task to perform, and on which device.
<br>
<br>
## Overview

When connected to other devices, you can make the GPIO device press specific buttons or read the state of a specific LED on one of the connected devices, by issuing the commands described in [Commands](#commands). 

The commands are implemented using Zephyr's Shell interface. After receiving a command, the GPIO device will set or get the value of the GPIO(s) corresponding to the selected button or LED(s) on the selected device(s), depending on the command.
<br>
<br>
## Requirements
The sample supports the following development kits:

```
nrf52840dk_nrf52840
```
### Hardware requirements
- nRF52840 DK as GPIO device
- Other development kits to connect to and control
- Cables to connect the pins
<br>

## Commands

**Press button**

<pre><code>lb set <i>device</i> <i>button</i></pre></code>

Example:

```
> lb set 1 3
```

Press button 3 on device 1.

**Read LED state**

<pre><code>lb get <i>device</i> <i>LED</i></pre></code>

Example:

```
> lb get 4 1
```

Read state of LED 1 on device 4.

**View all devices and LED states**

```
lb view
```
<br>

## Configuration

The table below shows which pins on the GPIO device should be connected to which buttons and LEDs on the connected devices. You must connect these pins on the GPIO device to the correct pins on the other device.

| LED/Button | Device 1 | Device 2 | Device 3 |
| --------- | --------- | --------- | --------- |
| Button 1  | P1.12  | P0.29  | P1.03  |
| Button 2  | P1.13  | P1.14  | P1.04  |
| Button 3  | P0.27  | P0.30  | P1.07  |
| Button 4  | P1.08  | P0.03  | P0.11  |
| LED 1  | P1.11  | P0.28  | P01.02  |
| LED 2  | P0.02  | P1.15  | P01.05  |
| LED 3  | P1.10  | P0.04  | P01.01  |
| LED 4  | P0.31  | P0.31  | P01.06  |

This configuration is set in [nrf52840dk_nrf52840.overlay](boards/nrf52840dk_nrf52840.overlay), and can be changed as desired. See [nRF52840 DK > Connector interface](https://infocenter.nordicsemi.com/topic/ug_nrf52840_dk/UG/dk/connector_if.html) for an overview of available GPIOs on nRF52840 DK.
<br>
<br>
## Building and running

Build the sample as described in [Building and programming an application](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/gs_programming.html)

