#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "logger.h"

DynamicJsonDocument settings(2048);

bool settings_read_config()
{
    Serial.println("loading settings");

    File fileSettings = LittleFS.open("/settings/settings.json", "r");

    if(!fileSettings)
    {
        Serial.print("Error: could not open settings.json");
        return false;
    }

    String fileContent;

    while(fileSettings.available())
    {
        String data = fileSettings.readString();
        fileContent += data;
        Serial.print(data);
    }
    Serial.print("\n(end)\n");

    fileSettings.close();

    if(deserializeJson(settings, fileContent) != DeserializationError::Ok)
    {
        Serial.println("Error: deser error!");
        return false;
    }
    Serial.println("Deserialization ok");
    return true;

}