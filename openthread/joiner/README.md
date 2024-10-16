<i>Latest nRF Connect version tested with: v2.6.2</i>

# OpenThread joiner

The OpenThread commissioner and joiner examples demonstrates how to commission a device using OpenThread Commissioner and Joiner roles.
<br>

## Requirements
The sample supports the following development kits:

```
nrf52840dk_nrf52840
nrf52833dk_nrf52833
nrf5340dk_nrf5340
```
You will need an additional OpenThread device to act as a commissioner.
<br>
<br>

## Building and running

Build and program the example as described in [Configuration and building](https://docs.nordicsemi.com/bundle/ncs-2.6.2/page/nrf/config_and_build.html)
<br>
<br>

## Testing

1. Program a DK with the [commissioner](/commissioner/) example
2. Wait for the commissioner to start up and add the joiner
3. Program a second DK with the joiner example
4. Wait for the joiner to join the network

```
*** Booting nRF Connect SDK v3.5.99-ncs1-2 ***
[00:00:00.022,155] <inf> ot_joiner: Start OpenThread joiner example
[00:00:00.022,216] <inf> ot_joiner: Starting joiner
[00:00:06.237,060] <inf> ot_joiner: Join success
[00:00:06.237,182] <inf> ot_joiner: Starting OpenThread
[00:00:07.901,367] <inf> ot_joiner: Connected to Thread network
[00:01:45.212,799] <inf> ot_joiner: Connected to Thread network
```
<br>
<br>