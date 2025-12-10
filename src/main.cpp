#include <Arduino.h>
#include <mcp4131.hpp>

#include <generic_resistive/generic_resistive_swc.hpp>
#include <headunit_swc.hpp>
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
    // Encoder Pin A triggered the interrupt.
    // Read Pin B to determine state change
    if (digitalRead(PIN_INPUT_ENCODER_B))
    {
        /* We have a CCW rotation */
        if (encoder_count > MIN_ENCODER_COUNT)
        {
            encoder_count -= 1;
        }
        return;
    }
    /* We have a CW rotation */
    if (encoder_count < MAX_ENCODER_COUNT)
    {
        encoder_count += 1;
    }
}

void encoder_button_interrupt_handler(void)
{
    detachInterrupt(PIN_INPUT_ENCODER_SW);
    encoder_flags |= ENCODER_FLAG_BUTTON_TIMER_STARTED_BM;
    button_press_start_time = millis();
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
    pinMode(PIN_INPUT_ENCODER_A, INPUT_PULLUP);
    pinMode(PIN_INPUT_ENCODER_B, INPUT_PULLUP);
    pinMode(PIN_INPUT_ENCODER_SW, INPUT_PULLUP);

    pinMode(PIN_OUTPUT_SWC_GND_EN, OUTPUT);
    digitalWrite(PIN_OUTPUT_SWC_GND_EN, LOW);

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

    // put your setup code here, to run once:
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    mcp4131.init(&SPI, SPI_CHIP_SEL_PIN);
    mcp4131.set_output_resistance(0); // Connect wiper to B-terminal

    switch (headunit_brand)
    {
    case HEADUNIT_GENERIC_RESISTIVE:
        generic_resistive_swc.init_generic_resistive_swc(&mcp4131, PIN_OUTPUT_SWC_GND_EN);
        /* Enable the test mode */
        //  generic_resistive_swc.run_loop_test();
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
