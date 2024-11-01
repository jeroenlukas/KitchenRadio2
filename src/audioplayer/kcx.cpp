#include <Arduino.h>
#include <HardwareSerial.h>

#include "configuration/config.h"
#include "audioplayer/kcx.h"



HardwareSerial serialKcx(2);

void kcx_connectPin(uint8_t level)
{
    if(level == 0)
    {
        pinMode(KCX_CONNECT, OUTPUT);
        digitalWrite(KCX_CONNECT, LOW);
    }
    else if (level == 1)
    {
        pinMode(KCX_CONNECT, INPUT); // High Z
    }
}

void kcx_init()
{
    serialKcx.begin(115200, SERIAL_8N1, KCX_RX, KCX_TX);
  
    // Send the power-off command early to not allow a phone to connect while starting
    serialKcx.write("AT+POWER_OFF\r\n");
}

void kcx_stop()
{
    serialKcx.write("AT+POWER_OFF\r\n");
}

void kcx_start()
{
    // Start the KCX receiver by pulling low 
    kcx_connectPin(0);
    delay(100);
    kcx_connectPin(1);
}