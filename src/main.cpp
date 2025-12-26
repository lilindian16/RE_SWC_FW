#include <Arduino.h>
#include <EEPROM.h>
#include <mcp4131.hpp>

/* CH32 core source */
#include <core_riscv_ch32yyxx.h>

/* Include the headunit_swc header first */
#include <headunit_swc.hpp>

/* Now include all the headunit_swc headers for each brand */
#include <generic_resistive/generic_resistive_swc.hpp>
#include <jvc/jvc_swc.hpp>
#include <kenwood/kenwood_swc.hpp>
#include <alpine/alpine_swc.hpp>
#include <pioneer/pioneer_swc.hpp>

// Encoder Pins
#define PIN_INPUT_ENCODER_A PA1
#define PIN_INPUT_ENCODER_B PA2
#define PIN_INPUT_ENCODER_SW PA3

// SWC Control Output
#define PIN_OUTPUT_SWC_GND_EN PB3
#define PIN_OUPUT_SWC_PUSH_PULL PB11

#define STATUS_LED_PIN PC15

#define SPI_CHIP_SEL_PIN PA4

/* Encoder state flags */
#define ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM (1 << 0)
#define ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM (1 << 1)
#define ENCODER_FLAG_ENCODER_BUTTON_HELD_BM (1 << 2)
#define ENCODER_FLAG_BUTTON_TIMER_STARTED_BM (1 << 3)

/* Ceiling and floor for encoder rotation counts */
#define MIN_ENCODER_COUNT -1
#define MAX_ENCODER_COUNT 1

// Button state thresholds
#define BUTTON_HELD_TIME_THRESHOLD_MS 500
#define BUTTON_RELEASED_TIME_THRESHOLD_MS 1000

/* EEPROM data addresses */
#define EEPROM_ADDRESS_HEADER 0x00
#define EEPROM_HEADER_SIZE_BYTES 4
#define EEPROM_ADDRESS_HEADUNIT_BRAND (EEPROM_ADDRESS_HEADER + EEPROM_HEADER_SIZE_BYTES)
#define EEPROM_HEADUNIT_BRAND_SIZE_BYTES 1

/* EEPROM data */
const uint8_t eeprom_header[] = {0xDE, 0xAD, 0xBE, 0xEF};

Headunit_Brand_t headunit_brand = HEADUNIT_ALPINE;

volatile int8_t encoder_count = 0;
volatile uint8_t encoder_flags = 0;
volatile uint32_t button_press_start_time = 0;

MCP4131 mcp4131;
Generic_Resistive_SWC generic_resistive_swc;
Kenwood_SWC kenwood_swc;
JVC_SWC jvc_swc;
Alpine_SWC alpine_swc;
Pioneer_SWC pioneer_swc;

void encoder_rotation_interrupt_handler(void)
{

    /* Encoder Pin A triggered the interrupt. Read Pin B to determine state change */
    if (digitalRead(PIN_INPUT_ENCODER_B))
    {
        /* We have a CCW rotation */
        __disable_irq();
        if (encoder_count > MIN_ENCODER_COUNT)
        {
            encoder_count -= 1;
        }
        /* Enable global interrupts */
        __enable_irq();
        return;
    }
    /* We have a CW rotation */
    __disable_irq();
    if (encoder_count < MAX_ENCODER_COUNT)
    {
        encoder_count += 1;
    }
    /* Enable global interrupts */
    __enable_irq();
}

void encoder_button_interrupt_handler(void)
{
    /* First, disable interrupt to avoid triggering again */
    detachInterrupt(PIN_INPUT_ENCODER_SW);
    /* Disable global interrupts to avoid race conditions */
    __disable_irq();
    /* Now we tell the main loop that the button has been pressed and set the initial time */
    encoder_flags |= ENCODER_FLAG_BUTTON_TIMER_STARTED_BM;
    button_press_start_time = millis();
    /* Finally, enable global interrupts */
    __enable_irq();
}

void on_encoder_rotation(bool cw_rotation)
{
    switch (headunit_brand)
    {
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

    default:
        break;
    }
}

void on_encoder_button_short_press(void)
{
    switch (headunit_brand)
    {
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

    default:
        break;
    }
}

void on_encoder_button_held(void)
{
    switch (headunit_brand)
    {
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

    default:
        break;
    }
}

void on_encoder_button_double_pressed(void)
{
    switch (headunit_brand)
    {
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

    default:
        break;
    }
}

void setup()
{
    /* Load EEPROM */
    EEPROM.begin();
    uint8_t index_counter = 0; // Temp variable for for-loops

    /* Check the EEPROM header on boot to check if it has been formatted */
    uint8_t current_eeprom_header[EEPROM_HEADER_SIZE_BYTES];
    for (index_counter = EEPROM_ADDRESS_HEADER; index_counter < EEPROM_HEADER_SIZE_BYTES; index_counter++)
    {
        current_eeprom_header[index_counter] = EEPROM.read(index_counter);
    }

    if (memcmp(eeprom_header, current_eeprom_header, EEPROM_HEADER_SIZE_BYTES) != 0) // Header not formatted
    {
        /* EEPROM is not set, we must set it up now */
        EEPROM.write(EEPROM_ADDRESS_HEADUNIT_BRAND, (uint8_t)HEADUNIT_GENERIC_RESISTIVE);
        /* Finally, set the EEPROM header */
        for (index_counter = EEPROM_ADDRESS_HEADER; index_counter < EEPROM_HEADER_SIZE_BYTES; index_counter++)
        {
            EEPROM.write(index_counter, eeprom_header[index_counter]);
        }

        /* We can commit it to the flash storage */
        EEPROM.commit(); // Call commit to ensure EEPROM is written to flash
    }

    else
    {
        headunit_brand = (Headunit_Brand_t)EEPROM.read(EEPROM_ADDRESS_HEADUNIT_BRAND);
    }

    pinMode(PIN_INPUT_ENCODER_A, INPUT_PULLUP);
    pinMode(PIN_INPUT_ENCODER_B, INPUT_PULLUP);
    pinMode(PIN_INPUT_ENCODER_SW, INPUT_PULLUP);

    pinMode(PIN_OUTPUT_SWC_GND_EN, OUTPUT);
    digitalWrite(PIN_OUTPUT_SWC_GND_EN, LOW);

    // put your setup code here, to run once:
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    /* Let's see if someone wants to change the headunit brand. This is done by holding the button down on boot */
    if (!digitalRead(PIN_INPUT_ENCODER_SW))
    {
        int held_time_miliseconds = 0;
        const int button_held_threshold_ms = 3000;
        while (!digitalRead(PIN_INPUT_ENCODER_SW) && held_time_miliseconds < button_held_threshold_ms)
        {
            held_time_miliseconds += 100;
            delay(100);
        }
        if (held_time_miliseconds >= button_held_threshold_ms)
        {
            /* User has held the button, turn on the LED until they release it */
            digitalWrite(STATUS_LED_PIN, HIGH);
            while (!digitalRead(PIN_INPUT_ENCODER_SW))
            {
                delay(100);
            }
            digitalWrite(STATUS_LED_PIN, LOW);
            /* Button released, now we either count presses or wait for a held input to indicate done */
            held_time_miliseconds = 0;
            uint8_t headunit_index = 0;
            bool user_completed = false;
            while (!user_completed)
            {
                if (!digitalRead(PIN_INPUT_ENCODER_SW))
                {
                    held_time_miliseconds = 0;
                    while (!digitalRead(PIN_INPUT_ENCODER_SW) && held_time_miliseconds < button_held_threshold_ms)
                    {
                        held_time_miliseconds += 100;
                        delay(100);
                    }

                    /* Button released. Decide if it was held or not */
                    if (held_time_miliseconds >= button_held_threshold_ms)
                    {
                        if (headunit_index < HEADUNIT_GENERIC_RESISTIVE || headunit_index >= (uint8_t)HEADUNIT_BRAND_ERROR)
                        {
                            headunit_index = (uint8_t)HEADUNIT_GENERIC_RESISTIVE;
                        }
                        /* Button held, let's save the headunit brand and break out */
                        EEPROM.write(EEPROM_ADDRESS_HEADUNIT_BRAND, headunit_index);
                        EEPROM.commit();
                        headunit_brand = (Headunit_Brand_t)headunit_index;
                        user_completed = true;
                        while (!digitalRead(PIN_INPUT_ENCODER_SW))
                        {
                            /* Flash to indicate user is done */
                            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
                            delay(50);
                        }
                        digitalWrite(STATUS_LED_PIN, LOW);
                        break;
                    }
                    else
                    {
                        headunit_index += 1;
                        if (headunit_index >= (uint8_t)HEADUNIT_BRAND_ERROR)
                        {
                            headunit_index = (uint8_t)HEADUNIT_BRAND_ERROR;
                        }
                        digitalWrite(STATUS_LED_PIN, HIGH);
                        delay(100);
                        digitalWrite(STATUS_LED_PIN, LOW);
                        delay(100);
                    }
                }
                delay(10);
            }
        }
    }

    delay(1000);

    /* Flash x times to show the current headunit selected */
    for (index_counter = 0; index_counter < (uint8_t)headunit_brand; index_counter++)
    {
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(250);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(250);
    }

    if (headunit_brand == HEADUNIT_ALPINE)
    {
        pinMode(PIN_OUPUT_SWC_PUSH_PULL, OUTPUT);
        digitalWrite(PIN_OUPUT_SWC_PUSH_PULL, LOW);
    }

    else
    {
        /* Alpine pin needs to be high-impedance when not in use */
        pinMode(PIN_OUPUT_SWC_PUSH_PULL, INPUT);
    }

    mcp4131.init(&SPI, SPI_CHIP_SEL_PIN);
    mcp4131.set_output_resistance(0); // Connect wiper to B-terminal

    switch (headunit_brand)
    {
    case HEADUNIT_GENERIC_RESISTIVE:
        generic_resistive_swc.init_generic_resistive_swc(&mcp4131, PIN_OUTPUT_SWC_GND_EN);
        break;

    case HEADUNIT_JVC:
        jvc_swc.init_jvc_swc(PIN_OUTPUT_SWC_GND_EN);
        break;

    case HEADUNIT_KENWOOD:
        kenwood_swc.init_kenwood_swc(PIN_OUTPUT_SWC_GND_EN);
        break;

    case HEADUNIT_ALPINE:
        mcp4131.disconnect_wiper(); // Ensure disconnected
        alpine_swc.init_alpine_swc(PIN_OUPUT_SWC_PUSH_PULL);
        break;

    case HEADUNIT_PIONEER:
        pioneer_swc.init_pioneer_swc(&mcp4131, PIN_OUTPUT_SWC_GND_EN);
        break;

    default:
        break;
    }

    attachInterrupt(PIN_INPUT_ENCODER_A, GPIO_Mode_IPU, encoder_rotation_interrupt_handler, EXTI_Mode_Interrupt,
                    EXTI_Trigger_Falling);
    attachInterrupt(PIN_INPUT_ENCODER_SW, GPIO_Mode_IPU, encoder_button_interrupt_handler, EXTI_Mode_Interrupt,
                    EXTI_Trigger_Falling);
}

void loop()
{
    /* Check if we have any input events */
    while (encoder_count || encoder_flags)
    {
        if (encoder_count != 0)
        {
            digitalWrite(STATUS_LED_PIN, HIGH);
            if (encoder_count > 0)
            {
                /* CW rotation */
                on_encoder_rotation(true);
                encoder_count--;
            }

            else
            {
                /* CCW rotation */
                on_encoder_rotation(false);
                encoder_count++;
            }
            digitalWrite(STATUS_LED_PIN, LOW);
        }

        if (encoder_flags & ENCODER_FLAG_BUTTON_TIMER_STARTED_BM)
        {
            uint32_t delta = 0;
            uint32_t current_time = millis();
            while (delta < BUTTON_HELD_TIME_THRESHOLD_MS && !digitalRead(PIN_INPUT_ENCODER_SW))
            {
                current_time = millis();
                delta = current_time - button_press_start_time;
                delay(10);
            }
            if (delta >= 500)
            {
                // If we reached here, the button was held
                encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_HELD_BM;
            }

            else
            {
                // Button has been released - we now wait to see if it gets pressed again
                uint32_t start_time = millis();
                current_time = millis();
                delta = 0;
                while (delta < BUTTON_RELEASED_TIME_THRESHOLD_MS && digitalRead(PIN_INPUT_ENCODER_SW))
                {
                    current_time = millis();
                    delta = current_time - start_time;
                    delay(10);
                }
                if (delta >= BUTTON_RELEASED_TIME_THRESHOLD_MS)
                {
                    // Button was not pressed again, register single click
                    encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM;
                }
                else
                {
                    // Button pressed again, register double press
                    encoder_flags |= ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM;
                }
            }

            // Finally, we can clear the flag
            encoder_flags &= ~(ENCODER_FLAG_BUTTON_TIMER_STARTED_BM);
            // and re-attach the ISR
            attachInterrupt(PIN_INPUT_ENCODER_SW, GPIO_Mode_IPU, encoder_button_interrupt_handler, EXTI_Mode_Interrupt,
                            EXTI_Trigger_Falling);
        }

        if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM)
        {
            encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_SINGLE_PRESS_BM); // Clear the flag
            digitalWrite(STATUS_LED_PIN, HIGH);
            on_encoder_button_short_press();
            digitalWrite(STATUS_LED_PIN, LOW);
        }

        if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_HELD_BM)
        {
            encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_HELD_BM); // Clear the flag
            digitalWrite(STATUS_LED_PIN, HIGH);
            on_encoder_button_held();
            digitalWrite(STATUS_LED_PIN, LOW);
        }

        if (encoder_flags & ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM)
        {
            if (headunit_brand == HEADUNIT_GENERIC_RESISTIVE)
            {
                Learning_Mode_State_t state = generic_resistive_swc.get_learning_mode_state();
                if (state == IDLE)
                {
                    generic_resistive_swc.on_button_double_press();
                }
                else if (state == COMPLETE)
                {
                    generic_resistive_swc.on_learning_mode_completed();
                    encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
                    digitalWrite(STATUS_LED_PIN, LOW);
                }
                else if (state == WAITING)
                {
                    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
                    delay(50);
                }
            }
            else
            {
                encoder_flags &= ~(ENCODER_FLAG_ENCODER_BUTTON_DOUBLE_PRESS_BM); // Clear the flag
                digitalWrite(STATUS_LED_PIN, HIGH);
                on_encoder_button_double_pressed();
                digitalWrite(STATUS_LED_PIN, LOW);
            }
        }
    }
}
