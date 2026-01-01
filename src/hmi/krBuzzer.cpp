#include <Arduino.h>

#include "hmi/krBuzzer.h"

#include "configuration/config.h"

void buzzer_init()
{
    pinMode(PIN_BUZZER, OUTPUT);

    buzzer_beep(200);
}

void buzzer_beep(uint16_t duration)
{    
    digitalWrite(PIN_BUZZER, HIGH);

    delay(duration);

    digitalWrite(PIN_BUZZER, LOW);
}