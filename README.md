# RE_SWC Firmware

This repo contains the Rotary Encoder Steering Wheel Controller firmware.

## Supported Headunits

The RW_SWC should be compatible with any headunit that supports learning mode / reassignment of analog steering wheel control input. The majority of android headunits support this.

Some named brand headunits should also be compatible. Check the owner's manual of your headunit to see if it supports learning mode / reassignment of steering wheel control input

## Requirements

- PlatformIO - I like running it as an extension in VSCode
- [JTAG2UPDI programmer](https://github.com/ElTangas/jtag2updi) - you can make one with an Arduino Nano
- RE_SWC Controller Kit

## New Headunits

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
