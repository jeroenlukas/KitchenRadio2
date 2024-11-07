#include <Arduino.h>
#include <HardwareSerial.h>

#include "configuration/config.h"
#include "audioplayer/kcx.h"
#include "configuration/constants.h"
#include "information/krInfo.h"


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

void kcx_getstatus()
{
    serialKcx.write("AT+PAUSE?\r\n");
}

void parseReply(String reply)
{
    Serial.println("Parse: [" + reply + "]");

    if(reply == "OK+PAUSE")
    {
        Serial.println("paused");
        information.audioPlayer.bluetoothMode = KCX_PAUSED;
    }
    else if(reply == "OK+PLAY")
    {
        Serial.println("playing");
        information.audioPlayer.bluetoothMode = KCX_PLAYING;
    }
    else if(reply == "no connect!")
    {
        Serial.println("not connected");
        information.audioPlayer.bluetoothMode = KCX_NOTCONNECTED;
    }
    else if(reply == "OK+POWEROFF_MODE")
    {
        information.audioPlayer.bluetoothMode = KCX_OFF;
    }
}

// Should be called from the mainloop often
void kcx_read()
{
    char rxBuf[32];
    String str;
    if(serialKcx.available())
    {
        serialKcx.readBytes(rxBuf, serialKcx.available()); //TODO change this into a state machine
        Serial.println((rxBuf));
        str = String(rxBuf);
        

        if(str.endsWith("\n"))
        {
            str.trim();
            parseReply(str);
        }


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