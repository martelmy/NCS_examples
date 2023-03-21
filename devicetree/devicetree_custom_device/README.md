# Devicetree - custom device (node and bindings)

This example demonstrates how to create a custom devicetree device with bindings.
<br>
## Overview

This example is mainly to show how to create a custom devicetree.
For this, you need to create a bindings (.yaml) file for your device. <u>It is important that the bindings file is located inside a dts/bindings subdirectory.</u> If not, the build system will not be able to find the bindings. 

In this example, the bindings file is [my-company,custom-device.yaml](dts/bindings/my-company,custom-device.yaml) and it looks like this:

```
compatible: "my-company,custom-node"
description: Custom devicetree node
properties:
  in-gpios:
    type: phandle-array
    description: input gpio pins
    required: true
  out-gpios:
    type: phandle-array
    description: output gpio pins
    required: true
```
This is a simple device with only two properties, one for input GPIOs and one for output GPIOs. They are both required, which means that they must be set in the devicetree node. This is done in a devicetree file, either .dts or .overlay. 

Here is the devicetree node for the devicetree binding above. This is from the overlay file [app.overlay](app.overlay):

```
/ {
    my_custom_device: my_custom_device {
        compatible = "my-company,custom-device";
        status = "okay";
        in-gpios = <&gpio0 3 GPIO_ACTIVE_HIGH>;
        out-gpios = <&gpio0 4 GPIO_ACTIVE_LOW>;
    };
};
```
The rest of the example is only for testing purposes, to see that the device is configured correctly and that we are able to set and get the values of the GPIOs.
<br>
<br>

## Requirements

The sample is tested with nRF52840 DK and nRF Connect SDK v2.3.0. To use it with other boards, make sure to use pins that are not by default used by something else.
<br>
<br>

## Further reading

Here are some links I recommend for more information about devicetree.
- [Introduction to devicetree](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/build/dts/intro.html)
- [Devicetree bindings](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/build/dts/bindings.html)
- [Devicetree HOWTOs](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/build/dts/howtos.html)
<br>
<br>

## Building and running

Build the sample as described in [Building and programming an application](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/programming.html)
<br>
<br>

## Testing
The testing section is not as relevant, as this example is mainly to show how you can create your own devicetree device, including the devicetree binding. I have nonetheless provided the steps to test it.
1. Build and program the example to a DK.
2. Connect the input and output GPIOs with a jumper cable.
3. Connect to a terminal emulator (e.g. using our [nRF Connect for VS Code extension](https://nrfconnect.github.io/vscode-nrf-connect/get_started/quick_debug.html#how-to-connect-to-the-terminal) or [PuTTY](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/testing.html#how-to-connect-with-putty)).
4. You should see log similar to this:
```
*** Booting Zephyr OS build v3.2.99-ncs2 ***
Starting custom devicetree node sample
----------------------------------
GPIO value: 0
Toggling output GPIO
GPIO value: 1
----------------------------------
GPIO value: 1
Toggling output GPIO
GPIO value: 0
----------------------------------
GPIO value: 0
Toggling output GPIO
GPIO value: 1
```