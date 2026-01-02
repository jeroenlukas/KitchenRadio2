#ifndef KR_INFO_H
#define KR_INFO_H

class Information {
    public:

        int hour;
        int minute;
        String timeShort;
        String dateMid;


        struct System
        {
            uint32_t uptimeSeconds;
            int8_t wifiRSSI;
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
            ///int r;
            //int g;
            //int b;
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
        };

        struct Weather
        {
            String stateShort;
            String stateLong;
            float temperature;
            double windSpeedKmh;
            int windSpeedBft;
            int stateCode;
            String icon;
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