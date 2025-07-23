# RE_SWC Firmware

This repo contains the Rotary Encoder Steering Wheel Controller firmware

## RE_SWC Functions:

- Volume +/-
- Button short press action
- Button long press action
- Learning mode for stereo activated with button double press

## Supported Headunits

The RW_SWC should be compatible with any headunit that supports learning mode/reassignment of analog steering wheel control input. The majority of android headunits support this.

Several named brand headunits should also be compatible. Check the owner's manual of your headunit to see if it supports learning mode/reassignment of steering wheel control input.

> [!TIP]
> View the [Compatibility List](https://docs.google.com/spreadsheets/d/1KuhRTHHPlsPpQyRziJOaQv1jJqykjcSSAFU2pcPYcbk/edit?usp=sharing) for the most up-to-date compatibility matrix

## Requirements For FW Dev & Flashing

- PlatformIO - I like running it as an extension in VSCode
- RE_SWC Controller Kit
- USB C data & power cable

The source for this project requires the Atmel megaAVR platformio core but uses only the AVR SDK functionality instead of the Arduino abstraction layer. Platformio handles all the set up. I find this much easier than using proprietary IDEs and configuring build tools etc

## Supporting New Headunits

The firmware for the RE_SWC can be configured as required. Steps to configure a new headunit:

1. Create headunit entry in platformio.ini file. Use the AndroidHeadunitGeneric env as a template. Replace <HEADUNIT_NAME> with the name of your headunit

```
[env:<HEADUNIT_NAME>]
build_type = release
build_flags =
    -D<HEADUNIT_NAME>
```

2. Enter configs for new headunit in _headunits_configs.h_ src file. Replace <HEADUNIT_NAME> with the name you defined in the platformio.ini file. Replace the xxx with the time required for each action (miliseconds)

```
#elif defined(<HEADUNIT_NAME>)
#define CONFIG_NAME                     "<HEADUNIT_NAME>"
#define SWC_OUTPUT_ENABLE_SHORT_HOLD_MS xxx
#define SWC_OUTPUT_ENABLE_LONG_HOLD_MS  xxx
#define SWC_OUTPUT_DISABLE_MS           xxx
```

You will now see be able to build your environment with PIO. For more information, visit the [PIO envs page](https://docs.platformio.org/en/latest/projectconf/sections/env/index.html)
