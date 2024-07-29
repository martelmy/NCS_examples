<i>Latest nRF Connect version tested with: v2.6.1</i>

# Hello World USB

This example demonstrates how to use CDC ACM USB for logging.

## Requirements
This sample should support any Nordic DK.

## Building and running

Build the sample as described in [Configuring and building an application](https://docs.nordicsemi.com/bundle/ncs-2.6.1/page/nrf/config_and_build/configuring_app/index.html)
<br>
<br>

## Testing
1. Connect a USB cable to the nRF USB connector on the DK.
2. Open the port for the USB in a serial terminal. 
<i>Make sure to use the correct device. It should show up as "My USB device".</i>

This is what the log should look:
```
*** Booting nRF Connect SDK v3.5.99-ncs1-1 ***
[00:00:00.342,651] <inf> app: Hello World
[00:00:00.442,779] <inf> app: Hello World
[00:00:00.542,877] <inf> app: Hello World
[00:00:00.642,974] <inf> app: Hello World
```