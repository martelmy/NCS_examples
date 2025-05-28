<i>Latest nRF Connect version tested with: v3.0.0</i>

# UART passthrough

This example demonstrates UART passthrough.

## Requirements
The sample supports the following development kits:

```
nrf52840dk/nrf52840
```
For other DKs, you must create board overlays similar to nrf52840dk_nrf52840.overlay.
You will need two DKs and two jumper cables.

## Configuration
Connect jumper cables on the DKs as follows:
|  Passthrough  | Hello world |
|:--------|--------:|
| P0.26 | P0.27 |
| P0.27 | P0.26 |

<i>To use other pins, change the overlays for both [hello_world_passthrough](hello_world_passthrough/boards/nrf52840dk_nrf52840.overlay) and [passhthrough](hello_world_passthrough/boards/nrf52840dk_nrf52840.overlay).</i>

## Testing
1. Connect the pins as described in [Configuration](#configuration)
2. Program the first DK with the [passthrough](/passthrough/) code
3. Program the second DK with the [hello world passthrough](/hello_world_passthrough/) code
4. Connect to the passthrough DK with a serial terminal emulator to see the logging
5. The log from the hello world passthrough DK should be sent to the passthrough DK and you should see it in the serial terminal emulator:

```
*** Booting nRF Connect SDK v3.0.0-3bfc46578e42 ***
*** Using Zephyr OS v4.0.99-a0e545cb437a ***
Hello World! 0
Hello World! 1
Hello World! 2
Hello World! 3
```