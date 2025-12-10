# RE_SWC Firmware

This repo contains the Rotary Encoder Steering Wheel Controller firmware. FW uses the Arduino platform. Being interrupt driven, the RE_SWC is very responsive to human input via the volume knob

## Compatible Headunits

As of HW V2, the RW_SWC is compatible with:

- Kenwood
- JVC
- Pioneer
- Alpine
- Android Headunits
- Any headunit that supports learning SWC via KEY 1 input

> [!TIP]
> View the [Compatibility List](https://docs.google.com/spreadsheets/d/1KuhRTHHPlsPpQyRziJOaQv1jJqykjcSSAFU2pcPYcbk/edit?usp=sharing) for the most up-to-date compatibility matrix

## SWC Functions

|        INPUT        | GENERIC RESISTIVE |      JVC       |    KENWOOD     |     ALPINE     |    Pioneer     |
| :-----------------: | :---------------: | :------------: | :------------: | :------------: | :------------: |
|   Volume Knob CW    |        Any        |    Volume +    |    Volume +    |    Volume +    |    Volume +    |
|   Volume Knob CCW   |        Any        |    Volume -    |    Volume -    |    Volume -    |    Volume -    |
| Button Short Press  |        Any        |      Mute      |      Mute      |      Mute      |      Mute      |
|  Button Long Press  |        Any        |   Next Track   |   Next Track   |   Next Track   |   Next Track   |
| Button Double Press |        Any        | Previous Track | Previous Track | Previous Track | Previous Track |

## Requirements For FW Dev & Flashing

- PlatformIO - I like running it as an extension in VSCode
- RE_SWC Controller Kit
- USB C data & power cable
