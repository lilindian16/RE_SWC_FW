# RE_SWC Firmware

This repo contains the Rotary Encoder Steering Wheel Controller firmware

## Compatible Headunits

The RW_SWC should be compatible with any headunit that supports learning mode/reassignment of analog steering wheel control input. The majority of android headunits support this.

Several named brand headunits should also be compatible. Check the owner's manual of your headunit to see if it supports learning mode/reassignment of steering wheel control input.

### Update - Firmware Version 2.0.0

FW version 2.0.0 makes the RE_SWC compatible with every JVC and Kenwood headunit. SWC is achieved with the blue and yellow stripe SWC input wire on the headunit.

> [!TIP]
> View the [Compatibility List](https://docs.google.com/spreadsheets/d/1KuhRTHHPlsPpQyRziJOaQv1jJqykjcSSAFU2pcPYcbk/edit?usp=sharing) for the most up-to-date compatibility matrix

## SWC Functions

|        INPUT        | GENERIC RESISTIVE |      JVC       |   KENWOOD    |
| :-----------------: | :---------------: | :------------: | :----------: |
|   Volume Knob CW    |        Any        |    Volume +    |   Volume +   |
|   Volume Knob CCW   |        Any        |    Volume -    |   Volume -   |
| Button Short Press  |        Any        |      Mute      | Play / Pause |
| Button Double Press |        Any        |   Next Track   |  Next Track  |
|     Button Held     |        Any        | Previous Track |     Mute     |

## Requirements For FW Dev & Flashing

- PlatformIO - I like running it as an extension in VSCode
- RE_SWC Controller Kit
- USB C data & power cable

The source for this project requires the Atmel megaAVR platformio core but uses only the AVR SDK functionality instead of the Arduino abstraction layer. Platformio handles all the toolchain set up. I find this much easier than using proprietary IDEs and configuring build tools etc
