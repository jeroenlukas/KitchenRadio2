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

            int bluetoothMode;
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
            uint8_t stationIndex;
            String stationName;

            uint8_t stationCount;
        };

        struct Weather
        {
            String stateShort;
            String stateLong;
            float temperature;
            double windSpeedKmh;
            int windSpeedBft;
            int stateCode;
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