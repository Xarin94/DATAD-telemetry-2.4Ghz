
# DATAD telmetry board 2.4GHz

This is the basic firmware of a teensy 4.0 to achieve a full duplex, programmable solution for a fast and long range datalink.

The goal is to achive a flexible platform for makers with little knowledge in programming, of a fleixible and reliable radio link.


## Features

- SX1280 based, on 2.4GHz small directional antennas can extend the range easily.
- Various I/O: usb serial, 4 serial (3 PWM capable), canbus, and I2C.
- Mavlink indipendent, created for the use on UAV's, but adaptable to every UART device. (transparent link, valid also for CANBUS)
- Indipendent RX and TX antennas, for optimal antenna placement.
- Full duplex: one sx1280 amplified at 2w maximum and one specific for reception.
- One sx1280 can be used for RX, TX and ranging.
- High speed and long ange FLRC modulation, or LoRa comminication selectable.
- Added the possibilty to read a PPM from a RC controller and inject in the autopilot via Mavlink messages.

