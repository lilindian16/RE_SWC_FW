# RE_SWC Firmware

This repo contains the Rotary Encoder Steering Wheel Controller firmware. FW uses the Arduino platform. Being interrupt driven, the RE_SWC is very responsive to human input via the volume knob

## Compatible Headunits

As of HW V2, the RW_SWC is compatible with:

- Headunits with Resistive Learning SWC (Generic Resistive)
- JVC
- Kenwood
- Alpine
- Pioneer & Sony
- USB HID (Android, Windows, MacOS etc)

> [!TIP]
> View the [Compatibility List](https://docs.google.com/spreadsheets/d/1KuhRTHHPlsPpQyRziJOaQv1jJqykjcSSAFU2pcPYcbk/edit?usp=sharing) for the most up-to-date compatibility matrix

## SWC Functions

|        INPUT        | GENERIC RESISTIVE |      JVC       |    KENWOOD     |     ALPINE     | Pioneer & Sony |    USB HID     |
| :-----------------: | :---------------: | :------------: | :------------: | :------------: | :------------: | :------------: |
|   Volume Knob CW    |        Any        |    Volume +    |    Volume +    |    Volume +    |    Volume +    |    Volume +    |
|   Volume Knob CCW   |        Any        |    Volume -    |    Volume -    |    Volume -    |    Volume -    |    Volume -    |
| Button Short Press  |        Any        |      Mute      |      Mute      |      Mute      |      Mute      |      Mute      |
|  Button Long Press  |        Any        |   Next Track   |   Next Track   |   Next Track   |   Next Track   |   Next Track   |
| Button Double Press |        Any        | Previous Track | Previous Track | Previous Track | Previous Track | Previous Track |

## Requirements For FW Dev & Flashing

- PlatformIO - I like running it as an extension in VSCode
- RE_SWC Controller Kit
- USB C data & power cable

## Configuring Headunit Brand

Headunit brand settings are stored in the (emulated) EEPROM of the chip. Users can set the brand of their headunit easily:

1. Hold down the volume knob button
2. Apply power to the RE_SWC controller via USB
3. Keep the button held until the status LED lights up
4. Press the volume knob button x times to enter your headunit brand (refer to the headunit brand index below). The RE_SWC status LED will blink to indicate the button has been pressed
5. Hold the volume knob button down until the status LED lights up to set the headunit brand
6. The RE_SWC controller will flash x times to indicate the brand it is programmed to

## Headunit Brand Index

1. Generic Resistive
2. JVC
3. Kenwood
4. Alpine
5. Pioneer & Sony
6. USB HID
