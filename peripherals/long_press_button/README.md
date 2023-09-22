<i>Latest nRF Connect version tested with: v2.4.2</i>

# Long press button

This example demonstrates how to implement long press for buttons.

## Requirements
This sample should support any Nordic DK .

## Building and running

Build the sample as described in [Building and programming an application](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/programming.html)
<br>
<br>

## User interface
<b>Button 1:</b><br>
- Press for less than one second for short press.
- Press for one second or more for long press.

## Testing
1. Turn on the development kit.
2. Try pressing button 1 for less than one second and more than one second and see the log output.

This is what the log should look like if you first press the button for less than one second and then one second or more:
```
*** Booting Zephyr OS build v3.3.99-ncs1-1 ***
[00:00:00.392,761] <inf> app: Long press button example started
[00:00:06.745,056] <inf> app: Short press
[00:00:11.579,315] <inf> app: Long press
```