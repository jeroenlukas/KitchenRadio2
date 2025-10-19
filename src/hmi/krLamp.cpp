#include <Arduino.h>
#include <Ticker.h>
#include "hmi/krLamp.h"
#include <NeoPixelBus.h>
#include "configuration/config.h"
#include "logger.h"
#include "information/krInfo.h"

const uint16_t PixelCount = LED_RING_NUM_LEDS; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = PIN_LED_RING;  // make sure to set this to the correct pin, ignored for Esp8266

String mitem_lamp_state_desc[2] = {"off", "on"};
String mitem_lamp_effecttype_desc[2] = {"off", "color fade"};

//#define colorSaturation 128

Ticker ticker_effect_100ms_ref;

NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);


bool lamp_state = false;


void ticker_effect_100ms()
{
    if(information.lamp.effect_type == LAMP_EFFECT_COLORFADE)
    {
        // Hue fade
        if((information.lamp.hue += information.lamp.effect_speed) > 1.0)
        {
            information.lamp.hue = 0.0;
        }
        lamp_sethue(information.lamp.hue);
    }
}

void lamp_init()
{
    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

    ticker_effect_100ms_ref.attach(0.1, ticker_effect_100ms);
    
    information.lamp.effect_type = LAMP_EFFECT_NONE; // No effect
    information.lamp.effect_speed = 0.001;

    information.lamp.hue = 0.2;
    information.lamp.saturation = 1.0;
    information.lamp.lightness = 0.3;
    
}

void lamp_update()
{
    RgbColor rgb(0,0,0);
    HslColor hsl(rgb);

    hsl.H = information.lamp.hue;
    hsl.S = information.lamp.saturation;
    hsl.L = information.lamp.lightness;

    for(int i = 0; i < LED_RING_NUM_LEDS; i++)
    {
        strip.SetPixelColor(i, hsl);
    }
    strip.Show();

    lamp_state = (information.lamp.lightness > 0.0);
    digitalWrite(LED_LAMP, lamp_state);
}

void lamp_toggle()
{
    if(lamp_state)
    {
        lamp_off();
        //information.lamp.effect = LAMP_EFFECT_NONE;
        information.lamp.state = false;
    }
    else
    {
        lamp_setlightness(0.5);
        lamp_update();
        information.lamp.state = true;
    }
}

void lamp_off()
{
    lamp_setlightness(0.0);    
}

// H, S values (0.0 - 1.0)
// L should be limited to between (0.0 - 0.5)
void lamp_sethue(float hue)
{    
    information.lamp.hue = constrain(hue, 0.0, 1.0);
    lamp_update();    
}

void lamp_setsaturation(float saturation)
{
    information.lamp.saturation = constrain(saturation, 0.0, 1.0);
    lamp_update();
}

void lamp_setlightness(float lightness)
{    
    information.lamp.lightness = constrain(lightness, 0.0, 0.5);
    lamp_update();
}

void lamp_seteffecttype(uint8_t effect)
{
    information.lamp.effect_type = effect;     
}

void lamp_seteffectspeed(float speed)
{

    information.lamp.effect_speed = constrain(speed, 0.000001, 0.5);     
}