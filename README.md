
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
An heat sink is suggested for continous trasmission, for example with a arduplane 50Hz telemetry on SR0 and EXTRA1, the remining at 10hz, no heatsink is needed.

### antennas and power

To correcly install the antennas on the drone keep in mind:
 - distance between the antennas is a must, higher is better.
 - you can use one vertically placed and one horizontally, different polarization will help with a loss of around 10dB.
 - my best choiche for mounting antennas is one on top of the other, with a 5dBi antnnenas the connector part is in a "shadow zone".
 - you can use RP-SMA cables to place the antennas where you want.
 - to power the board use a spare jst gh 4 pin connector to provide 5v to the board (in the old version do not use the 6-26v port, it will overheat and power off).

## Programming the Teensy 4.0

To program the main board you can use Arduino IDE with the needed libraries
The program in the same for the air and ground unit, with the paramenters you can select all the neededfunction of the radio.

The options are:
 - RadioType: 0 to select ground unit, 1 to select air unit. With this selection the rx and tx frequencies are automatically swapped and the RC in/out is automatically selected to have PPM in on the ground and PPM out on the air unit.
 - Rxfreq: the Reciving frequency of the ground unit and trasmitting on the air unit.
 - Txfreq: the opposite.
 - SPD: you can chose between 325,650,1300, those are MBPS, the best compromise is with a Correction of 3/4 for all the speeds, so the effective datarate is 25% than indicate. You can modify the correction param in the SX1280 initialization.
 - TxPower: The output power is accettable between 0 and -18dB from the sx1280, more than zero can damage the amplifier. The output is proportional the the input before the amplifer, at -18 the output is 20dB, at maximum 34,5db at 0 selected.
 - TelemID: unique code for the link, if multiple radios are on the same frequency you can use this paramenter so separate the messages, or to direct the messages to moltiple ports.
 - RCbusID: unique id to the PPM trasmission, same for the TelemID.
 - RCbusrate: delay in milliseconds between ppm packets, with two separate channels you can send lots of data without inferering with the telemetry downlink.
 - RCbuschannels: selectable number of channels needed, normally 16, encodable in the mavlink message only 8.
 - RCbuspin: the input/output pin of the PPM same on the air and ground unit normally, but you can customize it(check the exposed Serial ports, Serial3 is capable of high speed ppm).
 - RCouttype: 2 for PPM on the selected pin, 1 for the mavlink mix in the telemetry link ( SERVO_OVERRIDE function, like mission planner does).

After selecting all the prams you need to change from Serial to Serial1 functions between air and ground unit, depending on witch port you want to use.
There modification in needed in the first part of the program, in the reading part: the main function is Serial.available(), the second one and third are Serial.read().
the output is done on Serial and Serial1 at the same time.
