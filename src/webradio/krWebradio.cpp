#include <WiFi.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "webradio/krWebradio.h"
#include "audioplayer/krAudioPlayer.h"
#include "configuration/config.h"
#include "audioplayer/cbuf_ps.h"
#include "logger.h"
#include "information/krInfo.h"


WiFiClient webradio_client;

String webradio_read_stations();

//uint8_t webradio_stationIndex = 0;
//uint8_t webradio_numStations = 0;

bool dataPanic = false;

bool bufferedEnough = false;

bool webradio_isconnected()
{
    return (webradio_client.connected() > 0);
}

uint8_t webradio_get_num_stations()
{
    DynamicJsonDocument stations(2048);
    String fileContent = webradio_read_stations();

    if(deserializeJson(stations, fileContent) != DeserializationError::Ok)
    {
        Serial.println("Error: deser error!");
        return false;
    }

    return stations["stations"].size();
}

void webradio_open_url(char *host, char *path)
{
    if (webradio_client.connect(host, 80))
    {
        Serial.println("Connected now");
        log_debug("Connected");
    }

    Serial.print(host);
    Serial.println(path);

    circBuffer.flush();

    bufferedEnough = false;

    webradio_client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                          "Host: " + host + "\r\n" +
                          "Connection: close\r\n\r\n");
}

void webradio_stop()
{
    if(webradio_client.connected())
    {
        log_debug("Disconnecting");
        webradio_client.stop();
        audioplayer_flushbuffer();
    }
}

String webradio_read_stations()
{
//    DynamicJsonDocument stations(2048);

    File fileStations = LittleFS.open("/settings/stations.json", "r");

    if(!fileStations)
    {
        Serial.print("Error: could not open stations.json");
        return "";
    }

    String fileContent;

    while(fileStations.available())
    {
        String data = fileStations.readString();
        fileContent += data;
//        Serial.print(data);
    }
    Serial.print("\n(end)\n");

    fileStations.close();

    return fileContent;
}

bool webradio_open_station(uint8_t index)
{
    DynamicJsonDocument stations(2048);

    File fileStations = LittleFS.open("/settings/stations.json", "r");

    if(!fileStations)
    {
        Serial.print("Error: could not open stations.json");
        return false;
    }

    String fileContent;

    while(fileStations.available())
    {
        String data = fileStations.readString();
        fileContent += data;
//        Serial.print(data);
    }
    Serial.print("\n(end)\n");

    fileStations.close();

    if(deserializeJson(stations, fileContent) != DeserializationError::Ok)
    {
        Serial.println("Error: deser error!");
        return false;
    }
   

    String stationName = stations["stations"][index]["name"];
    String url = stations["stations"][index]["url"];
    Serial.println(stationName);
    Serial.println(url);

    information.webRadio.station_index = index;
    information.webRadio.station_name = stationName;
    // Split url in host and path
    // example: stream.bnr.nl/bnr_mp3_128_20
    uint16_t firstSlash = url.indexOf("/", 8);
    char host[64];
    char path[64];
    
    url.substring(0,firstSlash).toCharArray(host, 64);
    url.substring(firstSlash).toCharArray(path, 64);
    Serial.println("host: " + String(host));
    Serial.println("path: " + String(path));

    webradio_open_url(host, path);

    
    log_debug(stationName.c_str());

    return true;
}

bool webradio_buffered_enough(void)
{
    return bufferedEnough;
}

void webradio_handle_stream(void)
{
    if (webradio_client.available())
    {
        int bytes_read_from_stream = 0;

        #define BYTESTOGET  512

        if (circBuffer.room() > BYTESTOGET)
        {
          //  if(webradio_client.available() > 1000) Serial.printf("More than 1000 bytes available: %d", webradio_client.available());
            // Read either the maximum available (max 100) or the number of bytes to the next meata data interval
            bytes_read_from_stream = webradio_client.read((uint8_t *)readBuffer, min(BYTESTOGET, (int)webradio_client.available()));

            // If we get -1 here it means nothing could be read from the stream
            if (bytes_read_from_stream > 0)
            {
                // Add them to the circular buffer
                circBuffer.write(readBuffer, bytes_read_from_stream);

                // Some radio stations (eg BBC Radio 4!!!) limit the data to 92 bytes. Why?
                /*if (bytes_read_from_stream < 92 && bytesReadFromStream != bytesUntilmetaData)
                {
                    Serial.printf("Only wrote %db to circ buff\n", bytesReadFromStream);
                }*/

                if(circBuffer.available() > CONF_AUDIO_MIN_BYTES)
                {
                    bufferedEnough = true;
                }
            }
        }
        else
        {
            // There will be thousands of this message. Only for debugging.
            //Serial.println("Circ buff full.");
        }
    }
/*
    if (circBuffer.available())
    {
        // Does the VS1053 want any more data (yet)?
        if (player.data_request())
        {
            {
                // Read the data from the circuluar (ring) buffer
                int bytesRead = circBuffer.read((char *)mp3buff, 32);

                // If we didn't read the full 32 bytes, that's a worry
                if (bytesRead != 32)
                {
                    Serial.printf("Only read %d bytes from  circular buffer\n", bytesRead);
                }

                // Actually send the data to the VS1053
                player.playChunk(mp3buff, bytesRead);
            }
        }
    }*/
    
    return;
}