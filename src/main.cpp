#include <Arduino.h>
#include <EEPROM.h>
#include <mcp4131.hpp>

/* CH32 core source */
#include <core_riscv_ch32yyxx.h>

/* Include src for USB bootloader */
#include "config_bootloader.hpp"

/* Include all the headunit_swc headers for each brand */
#include <alpine/alpine_swc.hpp>
#include <generic_resistive/generic_resistive_swc.hpp>
#include <jvc/jvc_swc.hpp>
#include <kenwood/kenwood_swc.hpp>
#include <pioneer/pioneer_swc.hpp>
#include <sony/sony_swc.hpp>
#include <usb_hid/usb_hid_swc.hpp>

/* Encoder Pins */
#define PIN_INPUT_ENCODER_A  PA1
#define PIN_INPUT_ENCODER_B  PA2
#define PIN_INPUT_ENCODER_SW PA3

/* SWC Control Output */
#define PIN_OUTPUT_SWC_GND_EN   PB3
#define PIN_OUPUT_SWC_PUSH_PULL PB11

#define STATUS_LED_PIN PC15

#define SPI_CHIP_SEL_PIN PA4

/*
  Unused pins:
    - PA0
    - PB12
    - PC14
    - PB0
    - PB1
  Default for unused pins is input floating. All unused pins are to be tied to
  VCC or GND using internal PU or PD to reduce idle power draw.
*/

/* Encoder state flags */
#define ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM (1 << 0)
#define ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM (1 << 1)
#define ENCODER_FLAG_ENCODER_BUTTON_HELD_BM         (1 << 2)
#define ENCODER_FLAG_BUTTON_TIMER_STARTED_BM        (1 << 3)

/* Ceiling and floor for encoder rotation counts */
#define MIN_ENCODER_COUNT -1
#define MAX_ENCODER_COUNT 1

// Button state thresholds
#define BUTTON_HELD_TIME_THRESHOLD_MS     500
#define BUTTON_RELEASED_TIME_THRESHOLD_MS 500

/**
 * EEPROM data addresses
 * Note: Max size of EEPROM is 26 bytes with current library implementation
 */
#define EEPROM_ADDRESS_HEADER    0x00
#define EEPROM_HEADER_SIZE_BYTES 4
#define EEPROM_ADDRESS_OUTPUT_CONFIG                                           \
  (EEPROM_ADDRESS_HEADER + EEPROM_HEADER_SIZE_BYTES)
#define EEPROM_OUTPUT_CONFIG_SIZE_BYTES 13

/* EEPROM data */
const uint8_t eeprom_header[] = {0xDE, 0xAD, 0xDE, 0xED};

#define DEFAULT_OUTPUT_MAPPING                                                 \
  {                                                                            \
    .headunit_brand   = HEADUNIT_GENERIC_RESISTIVE,                            \
    .cw_rotate_output = VOLUME_UP, .ccw_rotate_output = VOLUME_DOWN,           \
    .button_short_press_output = MUTE, .button_long_press_output = NEXT_TRACK, \
    .button_double_press_output                  = PREVIOUS_TRACK,             \
    .cw_rotate_resistance_output                 = 1, /* 1k ohms */            \
        .ccw_rotate_resistance_output            = 2, /* 2k ohms */            \
        .button_short_press_resistance_output    = 5, /* 4k ohms */            \
        .button_long_press_resistance_output     = 7, /* 6k ohms */            \
        .button_double_press_resistance_output   = 9, /* 8k ohms */            \
        .resistance_output_hold_delay_time_units = 8, /* 8x10ms = 80ms */      \
        .resistance_output_off_delay_time_units  = 2  /* 2x10ms = 20ms */      \
  }

Output_Mapping_t output_mapping = DEFAULT_OUTPUT_MAPPING;

/* Encoder globals */
volatile int8_t  encoder_count = 0;
volatile uint8_t encoder_flags = 0;

/* Instances for each headunit model */
MCP4131               mcp4131;
Generic_Resistive_SWC generic_resistive_swc;
Kenwood_SWC           kenwood_swc;
JVC_SWC               jvc_swc;
Alpine_SWC            alpine_swc;
Pioneer_SWC           pioneer_swc;
USB_HID_SWC           usb_hid_swc;
Sony_SWC              sony_swc;

void encoder_rotation_interrupt_handler(void) {
  /* Disable global interrupts to avoid race conditions */
  __disable_irq();
  /* Encoder Pin A triggered the interrupt. Read Pin B to determine state change
   */
  if (digitalRead(PIN_INPUT_ENCODER_B)) {
    /* We have a CCW rotation */
    if (encoder_count > MIN_ENCODER_COUNT) {
      encoder_count -= 1;
    }
    /* Enable global interrupts */
    __enable_irq();
    return;
  }
  /* We have a CW rotation */
  if (encoder_count < MAX_ENCODER_COUNT) {
    encoder_count += 1;
  }
  /* Enable global interrupts */
  __enable_irq();
}

void encoder_button_interrupt_handler(void) {
  /* First, disable interrupt to avoid triggering again */
  detachInterrupt(PIN_INPUT_ENCODER_SW);
  /* Disable global interrupts to avoid race conditions */
  __disable_irq();
  /* Now we tell the main loop that the button has been pressed and set the
   * initial time */
  encoder_flags |= ENCODER_FLAG_BUTTON_TIMER_STARTED_BM;
  /* Finally, enable global interrupts */
  __enable_irq();
}

void on_encoder_rotation(bool cw_rotation) {
  switch (output_mapping.headunit_brand) {
  case HEADUNIT_GENERIC_RESISTIVE:
    generic_resistive_swc.on_encoder_rotation(cw_rotation);
    break;

  case HEADUNIT_JVC:
    jvc_swc.on_encoder_rotation(cw_rotation);
    break;

  case HEADUNIT_KENWOOD:
    kenwood_swc.on_encoder_rotation(cw_rotation);
    break;

  case HEADUNIT_ALPINE:
    alpine_swc.on_encoder_rotation(cw_rotation);
    break;

  case HEADUNIT_PIONEER:
    pioneer_swc.on_encoder_rotation(cw_rotation);
    break;

  case HEADUNIT_USB_HID:
    usb_hid_swc.on_encoder_rotation(cw_rotation);
    break;

  case HEADUNIT_SONY:
    sony_swc.on_encoder_rotation(cw_rotation);
    break;

  default:
    break;
  }
}

void on_encoder_button_short_press(void) {
  switch (output_mapping.headunit_brand) {
  case HEADUNIT_GENERIC_RESISTIVE:
    generic_resistive_swc.on_button_short_press();
    break;

  case HEADUNIT_JVC:
    jvc_swc.on_button_short_press();
    break;

  case HEADUNIT_KENWOOD:
    kenwood_swc.on_button_short_press();
    break;

  case HEADUNIT_ALPINE:
    alpine_swc.on_button_short_press();
    break;

  case HEADUNIT_PIONEER:
    pioneer_swc.on_button_short_press();
    break;

  case HEADUNIT_USB_HID:
    usb_hid_swc.on_button_short_press();
    break;

  case HEADUNIT_SONY:
    sony_swc.on_button_short_press();
    break;

  default:
    break;
  }
}

void on_encoder_button_held(void) {
  switch (output_mapping.headunit_brand) {
  case HEADUNIT_GENERIC_RESISTIVE:
    generic_resistive_swc.on_button_held();
    break;

  case HEADUNIT_JVC:
    jvc_swc.on_button_held();
    break;

  case HEADUNIT_KENWOOD:
    kenwood_swc.on_button_held();
    break;

  case HEADUNIT_ALPINE:
    alpine_swc.on_button_held();
    break;

  case HEADUNIT_PIONEER:
    pioneer_swc.on_button_held();
    break;

  case HEADUNIT_USB_HID:
    usb_hid_swc.on_button_held();
    break;

  case HEADUNIT_SONY:
    sony_swc.on_button_held();
    break;

  default:
    break;
  }
}

void on_encoder_button_double_pressed(void) {
  switch (output_mapping.headunit_brand) {
  case HEADUNIT_GENERIC_RESISTIVE:
    generic_resistive_swc.on_button_double_press();
    break;

  case HEADUNIT_JVC:
    jvc_swc.on_button_double_press();
    break;

  case HEADUNIT_KENWOOD:
    kenwood_swc.on_button_double_press();
    break;

  case HEADUNIT_ALPINE:
    alpine_swc.on_button_double_press();
    break;

  case HEADUNIT_PIONEER:
    pioneer_swc.on_button_double_press();
    break;

  case HEADUNIT_USB_HID:
    usb_hid_swc.on_button_double_press();
    break;

  case HEADUNIT_SONY:
    sony_swc.on_button_double_press();
    break;

  default:
    break;
  }
}

/**
 * Update an output mapping with an array. Useful for updating the output
 * mapping with EEPROM saved config
 * @param mapping Pointer to the output mapping struct
 * @param output_config Pointer to the output config the struct will update to
 * @param output_config_length Length of the output config buffer (must be the
 * same size as the output config struct)
 */
uint8_t update_output_mapping(Output_Mapping_t *mapping, uint8_t *output_config,
                              size_t output_config_length) {
  uint8_t ret = 0;
  if (output_config_length != sizeof(Output_Mapping_t)) {
    return ret;
  }
  mapping->headunit_brand               = (Headunit_Brand_t)output_config[0];
  mapping->cw_rotate_output             = (Output_Command_t)output_config[1];
  mapping->ccw_rotate_output            = (Output_Command_t)output_config[2];
  mapping->button_short_press_output    = (Output_Command_t)output_config[3];
  mapping->button_long_press_output     = (Output_Command_t)output_config[4];
  mapping->button_double_press_output   = (Output_Command_t)output_config[5];
  mapping->cw_rotate_resistance_output  = output_config[6];
  mapping->ccw_rotate_resistance_output = output_config[7];
  mapping->button_short_press_resistance_output    = output_config[8];
  mapping->button_long_press_resistance_output     = output_config[9];
  mapping->button_double_press_resistance_output   = output_config[10];
  mapping->resistance_output_hold_delay_time_units = output_config[11];
  mapping->resistance_output_off_delay_time_units  = output_config[12];
  return (sizeof(Output_Mapping_t));
}

/**
 * Convert an output mapping to an array. Useful for preparing to write
 * an output mapping to EEPROM as it converts all configs to bytes
 * @param mapping Pointer to output mapping you want to convert
 * @param output_buffer Pointer to an empty buffer for the output to be written
 * to
 * @param output_buffer_length Size of the empty buffer (must be the same length
 * as the output mapping struct)
 */
uint8_t convert_output_mapping_to_array(Output_Mapping_t *mapping,
                                        uint8_t          *output_buffer,
                                        size_t output_buffer_length) {
  uint8_t ret = 0;
  if (output_buffer_length != sizeof(Output_Mapping_t)) {
    return ret;
  }
  output_buffer[0]  = (uint8_t)mapping->headunit_brand;
  output_buffer[1]  = (uint8_t)mapping->cw_rotate_output;
  output_buffer[2]  = (uint8_t)mapping->ccw_rotate_output;
  output_buffer[3]  = (uint8_t)mapping->button_short_press_output;
  output_buffer[4]  = (uint8_t)mapping->button_long_press_output;
  output_buffer[5]  = (uint8_t)mapping->button_double_press_output;
  output_buffer[6]  = (uint8_t)mapping->cw_rotate_resistance_output;
  output_buffer[7]  = (uint8_t)mapping->ccw_rotate_resistance_output;
  output_buffer[8]  = (uint8_t)mapping->button_short_press_resistance_output;
  output_buffer[9]  = (uint8_t)mapping->button_long_press_resistance_output;
  output_buffer[10] = (uint8_t)mapping->button_double_press_resistance_output;
  output_buffer[11] = (uint8_t)mapping->resistance_output_hold_delay_time_units;
  output_buffer[12] = (uint8_t)mapping->resistance_output_off_delay_time_units;
  return sizeof(Output_Command_t);
}

void setup() {
  /* Load EEPROM */
  EEPROM.begin();
  uint8_t index_counter = 0; // Temp variable for for-loops
  uint8_t output_config_buffer[sizeof(Output_Mapping_t)];

  /* Check the EEPROM header on boot to check if it has been formatted */
  uint8_t current_eeprom_header[EEPROM_HEADER_SIZE_BYTES];
  for (index_counter = EEPROM_ADDRESS_HEADER;
       index_counter < EEPROM_HEADER_SIZE_BYTES; index_counter++) {
    current_eeprom_header[index_counter] = EEPROM.read(index_counter);
  }

  if (memcmp(eeprom_header, current_eeprom_header, EEPROM_HEADER_SIZE_BYTES) !=
      0) // Header not formatted
  {
    /* EEPROM is not set, we must set it up now */
    uint8_t output_mapping_bytes = convert_output_mapping_to_array(
        &output_mapping, output_config_buffer, sizeof(output_config_buffer));
    if (output_mapping_bytes) {
      for (size_t i = 0; i < sizeof(output_config_buffer); i++) {
        EEPROM.write(EEPROM_ADDRESS_OUTPUT_CONFIG + i, output_config_buffer[i]);
      }
    }
    /* Finally, set the EEPROM header */
    for (index_counter = EEPROM_ADDRESS_HEADER;
         index_counter < EEPROM_HEADER_SIZE_BYTES; index_counter++) {
      EEPROM.write(index_counter, eeprom_header[index_counter]);
    }
    /* We can commit it to the flash storage */
    EEPROM.commit(); // Call commit to ensure EEPROM is written to flash
  }

  else {
    /* Get config from EEPROM */
    for (size_t i = 0; i < sizeof(Output_Mapping_t); i++) {
      output_config_buffer[i] = EEPROM.read(EEPROM_ADDRESS_OUTPUT_CONFIG + i);
    }
    update_output_mapping(&output_mapping, output_config_buffer,
                          sizeof(output_config_buffer));
  }

  pinMode(PIN_INPUT_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_INPUT_ENCODER_B, INPUT_PULLUP);
  pinMode(PIN_INPUT_ENCODER_SW, INPUT_PULLUP);

  pinMode(PIN_OUTPUT_SWC_GND_EN, OUTPUT);
  digitalWrite(PIN_OUTPUT_SWC_GND_EN, LOW);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  /* All unused pins tied to either VCC or GND */
  pinMode(PA0, INPUT_PULLDOWN);
  pinMode(PB12, INPUT_PULLDOWN);
  pinMode(PC14, INPUT_PULLDOWN);
  pinMode(PB0, INPUT_PULLDOWN);
  pinMode(PB1, INPUT_PULLDOWN);

  /* Now we sample the button to check whether we enter the secondary bootloader
   */
  if (!digitalRead(PIN_INPUT_ENCODER_SW)) {
    int       held_time_miliseconds    = 0;
    const int button_held_threshold_ms = 3000;
    while (!digitalRead(PIN_INPUT_ENCODER_SW) &&
           held_time_miliseconds < button_held_threshold_ms) {
      held_time_miliseconds += 100;
      delay(100);
    }
    if (held_time_miliseconds >= button_held_threshold_ms) {
      /* User has held the button, turn on the LED until they release it */
      digitalWrite(STATUS_LED_PIN, HIGH);
      while (!digitalRead(PIN_INPUT_ENCODER_SW)) {
        delay(100);
      }
      digitalWrite(STATUS_LED_PIN, LOW);
      /* We have now activated the secondary bootloader */
      bool bootloader_complete = false;
      held_time_miliseconds    = 0;
      uint8_t headunit_index   = 0;

      init_usb_bootloader(&EEPROM);
      bool usb_bootloader_complete = false;

      while (!bootloader_complete) {
        /* Sample the button for configs */
        /* Button released, now we either count presses or wait for a held input
         * to indicate done */
        if (!digitalRead(PIN_INPUT_ENCODER_SW)) {
          digitalWrite(STATUS_LED_PIN, HIGH);
          held_time_miliseconds = 0;
          while (!digitalRead(PIN_INPUT_ENCODER_SW) &&
                 held_time_miliseconds < button_held_threshold_ms) {
            run_usb_bootloader_task((uint8_t)EEPROM_ADDRESS_OUTPUT_CONFIG);
            delay(10);
            held_time_miliseconds += 10;
          }
          /* Button released. Decide if it was held or not */
          if (held_time_miliseconds >= button_held_threshold_ms) {
            /* Button held, let's save the headunit brand and break out.Since
             * config is changed by user, we must use default settings or risk
             * writing settings that aren't supported by headunit*/
            if (headunit_index < HEADUNIT_GENERIC_RESISTIVE ||
                headunit_index >= (uint8_t)HEADUNIT_BRAND_ERROR) {
              headunit_index = (uint8_t)HEADUNIT_GENERIC_RESISTIVE;
            }
            Output_Mapping_t new_mapping = DEFAULT_OUTPUT_MAPPING;
            new_mapping.headunit_brand   = (Headunit_Brand_t)headunit_index;
            output_mapping               = new_mapping;
            uint8_t bytes_written        = convert_output_mapping_to_array(
                       &output_mapping, output_config_buffer,
                       sizeof(output_config_buffer));
            if (bytes_written) {
              for (size_t i = 0; i < sizeof(output_config_buffer); i++) {
                EEPROM.write(EEPROM_ADDRESS_OUTPUT_CONFIG + i,
                             output_config_buffer[i]);
              }
            }
            EEPROM.commit();
            while (!digitalRead(PIN_INPUT_ENCODER_SW)) {
              /* Flash to indicate user is done */
              digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
              delay(50);
            }
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(2500); // Wait to flash LED confirmation for user
            bootloader_complete = true;
          } else {
            digitalWrite(STATUS_LED_PIN, LOW);
            headunit_index += 1;
            if (headunit_index >= (uint8_t)HEADUNIT_BRAND_ERROR) {
              headunit_index = (uint8_t)HEADUNIT_BRAND_ERROR;
            }
          }
        }

        /* We also poll the USB CDC to see if we have commands to write to the
         * EEPROM */
        usb_bootloader_complete =
            run_usb_bootloader_task(EEPROM_ADDRESS_OUTPUT_CONFIG);
        if (usb_bootloader_complete) {
          bootloader_complete = true;
          EEPROM.commit(); // Call commit to ensure EEPROM is written to flash
          /* Wait for a few seconds and then reboot */
          delay(1000);
        }
      }
      NVIC_SystemReset();
    }
  }

  for (index_counter = 0;
       index_counter <
       (uint8_t)output_mapping.headunit_brand; // Use 1 flash as index 0
       index_counter++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(250);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(250);
  }

  if (output_mapping.headunit_brand == HEADUNIT_ALPINE) {
    pinMode(PIN_OUPUT_SWC_PUSH_PULL, OUTPUT);
    digitalWrite(PIN_OUPUT_SWC_PUSH_PULL, LOW);
  }

  else {
    /* Alpine pin needs to be high-impedance when not in use */
    pinMode(PIN_OUPUT_SWC_PUSH_PULL, INPUT);
  }

  mcp4131.init(&SPI, SPI_CHIP_SEL_PIN);
  mcp4131.set_output_resistance(0); // Connect wiper to B-terminal

  switch (output_mapping.headunit_brand) {
  case HEADUNIT_GENERIC_RESISTIVE:
    generic_resistive_swc.init_generic_resistive_swc(
        &mcp4131, PIN_OUTPUT_SWC_GND_EN, &output_mapping);
    break;

  case HEADUNIT_JVC:
    jvc_swc.init_jvc_swc(PIN_OUTPUT_SWC_GND_EN, &output_mapping);
    break;

  case HEADUNIT_KENWOOD:
    kenwood_swc.init_kenwood_swc(PIN_OUTPUT_SWC_GND_EN, &output_mapping);
    break;

  case HEADUNIT_ALPINE:
    mcp4131.disconnect_wiper(); // Ensure disconnected
    alpine_swc.init_alpine_swc(PIN_OUPUT_SWC_PUSH_PULL, &output_mapping);
    break;

  case HEADUNIT_PIONEER:
    pioneer_swc.init_pioneer_swc(&mcp4131, PIN_OUTPUT_SWC_GND_EN,
                                 &output_mapping);
    break;

  case HEADUNIT_USB_HID:
    usb_hid_swc.init_usb_hid_swc(&output_mapping);
    break;

  case HEADUNIT_SONY:
    sony_swc.init_sony_swc(&mcp4131, PIN_OUTPUT_SWC_GND_EN, &output_mapping);
    break;

  default:
    break;
  }

  attachInterrupt(PIN_INPUT_ENCODER_A, GPIO_Mode_IPU,
                  encoder_rotation_interrupt_handler, EXTI_Mode_Interrupt,
                  EXTI_Trigger_Falling);
  attachInterrupt(PIN_INPUT_ENCODER_SW, GPIO_Mode_IPU,
                  encoder_button_interrupt_handler, EXTI_Mode_Interrupt,
                  EXTI_Trigger_Falling);
}

void loop() {
  /* Check if we have any input events */
  while (encoder_count || encoder_flags) {
    if (encoder_count != 0) {
      digitalWrite(STATUS_LED_PIN, HIGH);
      if (encoder_count > 0) {
        /* CW rotation */
        on_encoder_rotation(true);
        encoder_count--;
      }

      else {
        /* CCW rotation */
        on_encoder_rotation(false);
        encoder_count++;
      }
      digitalWrite(STATUS_LED_PIN, LOW);
    }

    if (encoder_flags & ENCODER_FLAG_BUTTON_TIMER_STARTED_BM) {
      uint32_t delta_time_millis = 0;
      while (delta_time_millis < BUTTON_HELD_TIME_THRESHOLD_MS &&
             !digitalRead(PIN_INPUT_ENCODER_SW)) {
        delay(10);
        delta_time_millis += 10;
      }
      if (delta_time_millis >= 500) {
        // If we reached here, the button was held
        encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_HELD_BM;
      }

      else {
        // Button has been released - we now wait to see if it gets pressed
        // again
        delta_time_millis = 0;
        while (delta_time_millis < BUTTON_RELEASED_TIME_THRESHOLD_MS &&
               digitalRead(PIN_INPUT_ENCODER_SW)) {
          delay(10);
          delta_time_millis += 10;
        }
        if (delta_time_millis >= BUTTON_RELEASED_TIME_THRESHOLD_MS) {
          // Button was not pressed again, register single click
          encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM;
        } else {
          // Button pressed again, register double press
          encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM;
        }
      }

      // Finally, we can clear the flag
      encoder_flags &= ~(ENCODER_FLAG_BUTTON_TIMER_STARTED_BM);
      // and re-attach the ISR
      attachInterrupt(PIN_INPUT_ENCODER_SW, GPIO_Mode_IPU,
                      encoder_button_interrupt_handler, EXTI_Mode_Interrupt,
                      EXTI_Trigger_Falling);
    }

    if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM) {
      encoder_flags &=
          ~(ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM); // Clear the flag
      digitalWrite(STATUS_LED_PIN, HIGH);
      on_encoder_button_short_press();
      digitalWrite(STATUS_LED_PIN, LOW);
    }

    if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_HELD_BM) {
      encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_HELD_BM); // Clear the flag
      digitalWrite(STATUS_LED_PIN, HIGH);
      on_encoder_button_held();
      digitalWrite(STATUS_LED_PIN, LOW);
    }

    if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM) {
      if (output_mapping.headunit_brand == HEADUNIT_GENERIC_RESISTIVE) {
        Learning_Mode_State_t state =
            generic_resistive_swc.get_learning_mode_state();
        if (state == IDLE) {
          generic_resistive_swc.on_button_double_press();
        } else if (state == COMPLETE) {
          generic_resistive_swc.on_learning_mode_completed();
          encoder_flags &=
              ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
          digitalWrite(STATUS_LED_PIN, LOW);
        } else if (state == WAITING) {
          digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
          delay(50);
        }
      } else {
        encoder_flags &=
            ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
        digitalWrite(STATUS_LED_PIN, HIGH);
        on_encoder_button_double_pressed();
        digitalWrite(STATUS_LED_PIN, LOW);
      }
    }
  }
}