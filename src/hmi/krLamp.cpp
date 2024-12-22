#include <Arduino.h>
#include "hmi/krLamp.h"
#include <NeoPixelBus.h>
#include "configuration/config.h"
#include "logger.h"
#include "information/krInfo.h"

const uint16_t PixelCount = LED_RING_NUM_LEDS; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = PIN_LED_RING;  // make sure to set this to the correct pin, ignored for Esp8266

#define colorSaturation 128

// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);
/*
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor orange(colorSaturation, colorSaturation / 2, colorSaturation / 7);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);
HslColor hslOrange(orange);
*/

bool lamp_state = false;


void lamp_init()
{
    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

    
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
}

void lamp_toggle()
{
    if(lamp_state)
    {
        lamp_off();
    }
    else
    {
        lamp_setlightness(0.5);
        lamp_update();
    }
}

void lamp_off()
{
    //lamp_setcolor(0, 0, 0, 0);    
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





// DEPRECATED
void lamp_setcolor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    if (brightness == 0)
    {
        lamp_state = false;
    }
    else
    {
        lamp_state = true;
    } 
    digitalWrite(LED_LAMP, lamp_state);

    double brightness_div = (double)brightness / 255.0;

    r = r * brightness_div;
    g = g * brightness_div;
    b = b * brightness_div;

    Serial.print("Brightness_div:");
    Serial.print(brightness_div);
    Serial.print("R:");
    Serial.print(r);

    //information.lamp.r = r;
   /// information.lamp.g = g;
   // information.lamp.b = b;
   // information.lamp.brightness = brightness;
    

    RgbColor rgb(r, g, b);
    HslColor hsl(rgb);

    information.lamp.hue = hsl.H;
    information.lamp.saturation = hsl.S;
    information.lamp.lightness = brightness_div;

    lamp_update();
}