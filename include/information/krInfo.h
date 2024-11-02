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
        };
        


        System system;
        AudioPlayer audioPlayer;
        Webradio webRadio;
        Weather weather;

};

extern Information information;

void krInfoInitialize(void);

#endif