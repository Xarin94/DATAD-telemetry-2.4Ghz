
# DATAD telmetry board 2.4GHz

This is the basic firmware of a teensy 4.0 to achieve a full duplex, programmable solution for a fast and long range datalink.

The goal is to achive a flexible platform for makers with little knowledge in programming, of a fleixible and reliable radio link.
[Arduplane Forum](https://discuss.ardupilot.org/t/datad-2-4ghz-telemetry-link-2-w-open-source/104818)


## Features

- SX1280 based, on 2.4GHz small directional antennas can extend the range easily.
- Various I/O: usb serial, 4 serial (3 PWM capable), canbus, and I2C.
- Mavlink indipendent, created for the use on UAV's, but adaptable to every UART device. (transparent link, valid also for CANBUS)
- Indipendent RX and TX antennas, for optimal antenna placement.
- Full duplex: one sx1280 amplified at 2w maximum and one specific for reception.
- One sx1280 can be used for RX, TX and ranging.
- High speed and long ange FLRC modulation, or LoRa comminication selectable.
- Added the possibilty to read a PPM from a RC controller and inject in the autopilot via Mavlink messages.



## Usage
To avoid any damage on the reciving end, when using high power output DO NOT point antennas direcly to each other! SX1280 maximum input power is 6dB not 34!
In future we will implement a power output dependent on the RSSI from the other radio, but the risk of burning the reciving end remains.

### Install

To correcly install the antennas on the drone keep in mind:
 - distance between the antennas is a must, higher is better.
 - you can use one vertically placed and one horizontally, different polarization will help with a loss of around 10dB.
 - my best choiche for mounting antennas is one on top of the other, with a 5dBi antnnenas the connector part is in a "shadow zone".
 - you can use RP-SMA cables to place the antennas where you want.
 - to power the board use a spare jst gh 4 pin connector to provide 5v to the board (in the old version do not use the 6-26v port, it will overheat and power off).

