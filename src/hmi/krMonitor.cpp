#include <Arduino.h>
#include "hmi/krMonitor.h"
#include "configuration/config.h"
#include "configuration/constants.h"

#include "audioplayer/krAudioplayer.h"
#include "audioplayer/kcx.h"
#include "information/krWeather.h"
#include "information/krTime.h"


/*
void mon_parseCommand(String command);

void mon_receiveCommand()
{
    if(Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();
        Serial.println("Command: [" + command + "]" );
        
        mon_parseCommand((command));
    }
}

void mon_parseCommand(String command)
{
    if(command == "off")    
        audioplayer_set_soundmode(SOUNDMODE_OFF);
    
    else if(command == "radio")
        audioplayer_set_soundmode(SOUNDMODE_WEBRADIO);

    else if(command == "weather")
        weather_retrieve();

    else if(command == "reset")
        ESP.restart();

    else if(command == "kcx_stop")
        kcx_stop();

        
    else
    {
        Serial.println("Unknown command");
    }

}*/