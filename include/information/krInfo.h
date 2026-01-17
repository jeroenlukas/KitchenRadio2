#ifndef KR_INFO_H
#define KR_INFO_H

class Information {
    public:

        int hour;
        int minute;
        String timeShort;
        String dateMid;

        String device_name;

        struct System
        {
            uint32_t uptimeSeconds;
            int8_t wifiRSSI;
            int8_t bluetoothRSSI;
            String IPAddress;
            uint8_t ldr;
        };
        struct AudioPlayer
        {
            int channels;
            int bitRate;
            int sampleRate;

            int volume;

            int bass;
            int treble;

            int bluetoothMode;

            String bluetoothTitle;
            String bluetoothArtist;
        };

        struct Lamp
        {
            bool state;
            float lightness;
            float hue; 
            float saturation;
            uint8_t effect_type;
            float effect_speed;
        };

        struct Webradio
        {
            uint8_t station_index;
            String station_name;
            uint8_t station_count;
            uint8_t buffer_pct;
        };

        struct Weather
        {
            String stateShort;
            String stateLong;
            float temperature;
            double windSpeedKmh;
            int windSpeedBft;
            int stateCode;
            int pressure;
            float temperature_feelslike;
            int humidity;
            time_t sunrise;
            time_t sunset;
            String icon;
            String sunrise_str;
            String sunset_str;

        };
        


        System system;
        AudioPlayer audioPlayer;
        Lamp lamp;
        Webradio webRadio;
        Weather weather;

};

extern Information information;

void krInfoInitialize(void);

#endif