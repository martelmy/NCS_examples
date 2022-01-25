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

<pre><code>lb get <i>device</i> <i>button</i></pre></code>

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

| LED/Button | Device 1 | Device 2 | Device 3 | Device 4 |
| --------- | --------- | --------- | --------- | --------- |
| Button 1  | P1.01  | P0.23  | P0.30  | P1.10  |
| Button 2  | P1.09  | P1.00  | P0.31  | P1.08  |
| Button 3  | P1.06  | P0.18  | P0.26  | P1.15  |
| Button 4  | P1.07  | P0.17  | P0.27  | P0.02  |
| LED 1  | P1.03  | P0.21  | P0.29  | P1.12  |
| LED 2  | P1.02  | P0.22  | P0.28  | P1.11  |
| LED 3  | P1.05  | P0.19  | P0.04  | P1.14  |
| LED 4  | P1.04  | P0.20  | P0.03  | P1.13  |

This configuration is set in [nrf52840dk_nrf52840.overlay](boards/nrf52840dk_nrf52840.overlay), and can be changed as desired.
<br>
<br>
## Building and running

Build the sample as described in [Building and programming an application](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/gs_programming.html)

