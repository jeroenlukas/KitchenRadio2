#include <WiFi.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "information/krInfo.h"
#include "configuration/config.h"
#include "logger.h"

//#include "configMisc.h"

int weather_statecode_to_glyph(int statecode);
int weather_icon_to_glyph(String icon);
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
        String weather_icon = doc["weather"][0]["icon"];
        information.weather.icon = weather_icon;
        //information.weather.windSpeedBft = doc["main"]

        Serial.println("\nWeather icon:" + information.weather.icon);
        
        
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

int weather_icon_to_glyph(String icon)
{
    if(icon == "01d") return 73; // Clear
    if(icon == "01n") return 78;

    if(icon == "02d") return 34; // Few clouds
    if(icon == "02n") return 35;

    if(icon == "03d") return 33; // Scattered clouds
    if(icon == "03n") return 33;

    if(icon == "04d") return 33; // Broken clouds
    if(icon == "04n") return 33; 

    if(icon == "09d") return 36; // Shower rain (heavier than 'rain')
    if(icon == "09n") return 36;

    if(icon == "10d") return 37; // Rain
    if(icon == "10n") return 38;

    if(icon == "11d") return 70; // Thunderstorm
    if(icon == "11n") return 70;

    if(icon == "13d") return 54; // Snow
    if(icon == "13n") return 56;

    if(icon == "50d") return 63; // Mist
    if(icon == "50n") return 65;
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
