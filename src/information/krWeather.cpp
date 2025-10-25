#include <WiFi.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "information/krInfo.h"
#include "configuration/config.h"
#include "logger.h"

//#include "configMisc.h"

int weather_statecode_to_glyph(int statecode);
int weather_windkmh_to_beaufort(double wind_kmh);

HTTPClient http;

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=Zierikzee,nl&units=metric&lang=nl&APPID=";
const String key = CONFIG_OPENWEATHER_KEY;

float weather_temperature = 0.0;
float windspeed_kmh = 0.0;
float windspeed = 0.0;

int weather_temperature_int = 0;


bool weather_retrieve()
{
    bool ret = false;
    http.begin(endpoint + key);

    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);

        deserializeJson(doc, payload);
        String weather_type = doc["weather"][0]["description"];
        Serial.print(payload);
        information.weather.stateShort = weather_type;
        information.weather.temperature = doc["main"]["temp"];        
        information.weather.windSpeedKmh = ((double)(doc["wind"]["speed"])) ;
        information.weather.windSpeedBft = weather_windkmh_to_beaufort(information.weather.windSpeedKmh);
        information.weather.stateCode = (int)(doc["weather"][0]["id"]);
        //information.weather.windSpeedBft = doc["main"]

        Serial.println("\nkm/h:" + String(information.weather.windSpeedKmh));
        Serial.println("\nbft: " + String(information.weather.windSpeedBft));
        
        ret = true;
    }
    else
        Serial.println("weather_retrieve: Error on HTTP request");

    http.end();
    
    return ret;
}

int weather_statecode_to_glyph(int statecode)
{
    //log_debug("Statecode : " + String(statecode));

    int glyph = 0;

    switch(statecode)
    {
        case 200:
        case 201:
        case 202:
        case 210:
        case 211:
        case 212:
        case 221:
        case 230:
        case 231:
        case 232:
            glyph = 70;   // Thunderstorm
            break;
        case 300:
        case 301:
        case 302:
        case 310:
        case 311:
        case 312:
        case 313:
        case 314:
        case 321:
            glyph = 39;   // Drizzle
            break;
        case 500:
        case 501:
        case 502:
        case 503:
        case 504:
        case 511:
        case 520:
        case 521:
        case 522:
        case 531:
            glyph = 36;  // Rain
            break;
        case 600:
        case 601:
        case 602:
        case 611:
        case 612:
        case 613:
        case 615:
        case 616:
        case 620:
        case 621:
        case 622:
            glyph = 57; // Snow
            break;
        case 701:
        case 711:
        case 741:
            glyph = 63; // Mist
            break;
        case 800:
            glyph = 73; // Clear
            break;
        case 801:
        case 802:
        case 803:
        case 804:
            glyph = 33; // Clouds
            break;
        default:
            glyph = 96; // Unknown
            break;

    }

    //log_debug("Glyph: " + String(glyph));

    return glyph;

    
}

int weather_windkmh_to_beaufort(double wind_kmh)
{
    if(wind_kmh < 1.0) return 0;
    if(wind_kmh <= 5.0) return 1;
    if(wind_kmh <= 11.0) return 2;
    if(wind_kmh <= 19.0) return 3;
    if(wind_kmh <= 28.0) return 4;
    if(wind_kmh <= 38.0) return 5;
    if(wind_kmh <= 49.0) return 6;
    if(wind_kmh <= 61.0) return 7;
    if(wind_kmh <= 74.0) return 8;
    if(wind_kmh <= 88.0) return 9;
    if(wind_kmh <= 102.0) return 10;
    if(wind_kmh <= 117.0) return 11;
    if(wind_kmh > 118.0) return 12;
}
