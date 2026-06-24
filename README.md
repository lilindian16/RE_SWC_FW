# RE_SWC Firmware

This repo contains the Rotary Encoder Steering Wheel Controller firmware. FW uses the Arduino platform. Being interrupt driven, the RE_SWC is very responsive to human input via the volume knob

## Revision History

### v4.0.0

- User output mapping supported via secondary (config) bootloader
- Improved responsiveness of Generic Resistive output

### v3.0.3

- Fixed and updated the MCP4131 driver
- Added delay to Gen Res output turn off. This allows forces a gap between Gen Res output change

### v3.0.2

- Added separate Sony headunit and required configs

### v3.0.1

- Improved JVC UX
- Set unused IO to INPUT_PULLDOWN to reduce quiescent current

## Compatible Headunits

As of HW V2, the RW_SWC is compatible with:

- Headunits with Resistive Learning SWC (Generic Resistive)
- JVC
- Kenwood
- Alpine
- Pioneer
- USB HID (Android, Windows, MacOS etc)
- Sony

> [!TIP]
> View the [Compatibility List](https://docs.google.com/spreadsheets/d/1KuhRTHHPlsPpQyRziJOaQv1jJqykjcSSAFU2pcPYcbk/edit?usp=sharing) for the most up-to-date compatibility matrix

## Default Output Mapping

|                    INPUT                     |        OUTPUT        |
| :------------------------------------------: | :------------------: |
|     Volume Knob Clockwise Rotation (CW)      |       Volume+        |
| Volume Knob Counter Clockwise Rotation (CCW) |       Volume-        |
|              Button Short Press              |       Mute/ATT       |
|              Button Long Press               |      Next Track      |
|   Button Double Press (Generic Resistive)    | Enable Learning Mode |
|         Button Double Press (Others)         |    Previous Track    |

## Output Function Support

FW v_4.0.0

|     OUTPUT     |        JVC         |      KENWOOD       |       ALPINE       |      Pioneer       |      USB HID       |        SONY        |
| :------------: | :----------------: | :----------------: | :----------------: | :----------------: | :----------------: | :----------------: |
|    Volume+     | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
|    Volume-     | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
|    Mute/ATT    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
|   Next Track   | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Previous Track | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
|   Play/Pause   |        :x:         | :heavy_check_mark: |        :x:         |        :x:         | :heavy_check_mark: |        :x:         |
| Change Source  |        :x:         |        :x:         |        :x:         | :heavy_check_mark: |        :x:         |        :x:         |

## Configuring Headunit Brand

### Standalone method

Headunit brand settings are stored in the (emulated) EEPROM of the chip. Users can set the brand of their headunit easily:

1. Hold down the volume knob button
2. Apply power to the RE_SWC controller via USB
3. Keep the button held until the status LED lights up
4. Press the volume knob button x times to enter your headunit brand (refer to the headunit brand index below). The RE_SWC status LED will blink to indicate the button has been pressed
5. Hold the volume knob button down until the status LED lights up to set the headunit brand
6. The RE_SWC controller will flash x times to indicate the brand it is programmed to

When using this method to set the headunit brand, the default output mapping will be used

## Headunit Brand Index

1. Generic Resistive
2. JVC
3. Kenwood
4. Alpine
5. Pioneer
6. USB HID
7. Sony

## Requirements For FW Dev & Flashing

- PlatformIO - I like running it as an extension in VSCode
- RE_SWC Controller Kit
- USB C data & power cable

## Contributions

Pull requests are more than welcome!
To ensure code formatting stays consistent, use the pre-commit hook before making any commits from your fork to this branch.

General workflow before a pull request:

1. Create a python venv in this working directory
2. Install pre-commit package with `pip install pre-commit`
3. (If required) install the pre-commit hook using `pre-commit install`
4. Run pre-commit hook against all files with `pre-commit run --all-files`

Pre-commit will format additional code according to the `.clang-format` file spec located in the root of this repo
