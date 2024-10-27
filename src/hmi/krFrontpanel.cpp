#include <Arduino.h>
//#include <RotaryEncoder.h>
#include <BfButtonManager.h>
#include <BfButton.h>
#include <RotaryEncoder.h>
#include "configuration/config.h"
#include "hmi/krFrontPanel.h"
#include "flags.h"

// To calibrate button ADC values
//#define CAL_BUTTONS

int adc_pot_vol = 0;

int prev_adc_pot_vol = 0;

int front_pot_vol = 0;

bool prev_button_encoder = true;


BfButtonManager buttonmanager(BUTTONS, 6);
BfButton btn_off(BfButton::ANALOG_BUTTON_ARRAY, 0);
BfButton btn_radio(BfButton::ANALOG_BUTTON_ARRAY, 1);
BfButton btn_bluetooth(BfButton::ANALOG_BUTTON_ARRAY, 2);
BfButton btn_system(BfButton::ANALOG_BUTTON_ARRAY, 3);
BfButton btn_alarm(BfButton::ANALOG_BUTTON_ARRAY, 4);
BfButton btn_lamp(BfButton::ANALOG_BUTTON_ARRAY, 5);

RotaryEncoder encoder(ROTARY_A, ROTARY_B, RotaryEncoder::LatchMode::FOUR3);

void button_press_handler(BfButton *btn, BfButton::press_pattern_t pattern)
{
    Serial.print(btn->getID());
   switch(btn->getID())
   {
    case 0:
        flags.frontPanel.buttonOffPressed = true;
        break;
    case 1:
        flags.frontPanel.buttonRadioPressed = true;
        break;
    case 2:
        flags.frontPanel.buttonBluetoothPressed = true;
        break;
    case 3:
        flags.frontPanel.buttonSystemPressed = true;
        break;
    case 4:
        flags.frontPanel.buttonAlarmPressed = true;
        break;
    case 5:
        flags.frontPanel.buttonLampPressed = true;
        break;
    default:
        break;
   }

   /*  Serial.print(btn->getID());
  switch (pattern)  {
    case BfButton::SINGLE_PRESS:
      Serial.println(" pressed.");
      break;
    case BfButton::DOUBLE_PRESS:
      Serial.println(" double pressed.");
      break;
    case BfButton::LONG_PRESS:
      Serial.println(" long pressed.");
      break;
  }*/
}


void front_led_on(uint8_t led)
{
    digitalWrite(led, 1);
}

void front_led_off(uint8_t led)
{
    digitalWrite(led, 0);
}

void frontpanel_setup()
{
    // LEDs
    pinMode(LED_WEBRADIO, OUTPUT);
    pinMode(LED_BLUETOOTH, OUTPUT);
    pinMode(LED_ALARM, OUTPUT);
    pinMode(LED_LAMP, OUTPUT);
    

    // Buttons
    btn_off.onPress(button_press_handler);
    btn_off.onDoublePress(button_press_handler);
    btn_off.onPressFor(button_press_handler, 1000);
    buttonmanager.addButton(&btn_off, BTN_ADC_OFF_MIN, BTN_ADC_OFF_MAX);

    btn_radio.onPress(button_press_handler);
    btn_radio.onDoublePress(button_press_handler);
    btn_radio.onPressFor(button_press_handler, 1000);
    buttonmanager.addButton(&btn_radio, BTN_ADC_RADIO_MIN, BTN_ADC_RADIO_MAX);

    btn_bluetooth.onPress(button_press_handler);
    btn_bluetooth.onDoublePress(button_press_handler);
    btn_bluetooth.onPressFor(button_press_handler, 1000);
    buttonmanager.addButton(&btn_bluetooth, BTN_ADC_BLUETOOTH_MIN, BTN_ADC_BLUETOOTH_MAX);

    btn_system.onPress(button_press_handler);
    btn_system.onDoublePress(button_press_handler);
    btn_system.onPressFor(button_press_handler, 1000);
    buttonmanager.addButton(&btn_system, BTN_ADC_SYSTEM_MIN, BTN_ADC_SYSTEM_MAX);

    btn_alarm.onPress(button_press_handler);
    btn_alarm.onDoublePress(button_press_handler);
    btn_alarm.onPressFor(button_press_handler, 1000);
    buttonmanager.addButton(&btn_alarm, BTN_ADC_ALARM_MIN,BTN_ADC_ALARM_MAX);

    btn_lamp.onPress(button_press_handler);
    btn_lamp.onDoublePress(button_press_handler);
    btn_lamp.onPressFor(button_press_handler, 1000);
    buttonmanager.addButton(&btn_lamp,BTN_ADC_LAMP_MIN,BTN_ADC_LAMP_MAX); 
    
    buttonmanager.begin();
}


void front_multibuttons_loop()
{
    buttonmanager.loop();
}

void front_read_buttons()
{
    // Encoder switch
    if (digitalRead(BUTTON_ENCODER) < prev_button_encoder)
    {
        flags.frontPanel.encoderButtonPressed = true;
    }



    // Multibuttons
    #ifdef CAL_BUTTONS
    static uint16_t reading;
    static uint32_t sum;
    static uint32_t avg;
    static unsigned int i = 0;
    const unsigned int pin = BUTTONS;
    reading = BfButtonManager::printReading(pin);

    if (reading > 100) { // button pressed
        sum += reading;
        if (i == 4) {
        avg = sum / 5;
        Serial.print("Avarage Reading: ");
        Serial.println(avg);
        sum = 0;
        }
        i++;
        if (i > 4) i = 0;
    } else { // button released
        sum = 0;
        i = 0;
    }
    #endif

    prev_button_encoder = digitalRead(BUTTON_ENCODER);
}

void front_read_encoder()
{
    static int pos = 0;
    encoder.tick();

    int newPos = encoder.getPosition();
    if (pos != newPos)
    {
        if ((int)(encoder.getDirection()) == 1)
        {
            //f_front_encoder_turn_right = true;
            flags.frontPanel.encoderTurnRight = true;
        }
        else
        {
            //f_front_encoder_turn_left = true;
            flags.frontPanel.encoderTurnLeft = true;
        }
        pos = newPos;
    }
}

void front_read_pots()
{
    adc_pot_vol = (4095 - analogRead(POT_VOLUME)) >> 5;

  //  Serial.print(adc_pot_vol);

    if (adc_pot_vol < (prev_adc_pot_vol - POT_HYST) || adc_pot_vol > (prev_adc_pot_vol + POT_HYST))
    {
        flags.frontPanel.volumePotChanged = true;
        front_pot_vol = adc_pot_vol;
    }

    prev_adc_pot_vol = adc_pot_vol;
    
}