#ifndef KR_INFO_H
#define KR_INFO_H

class Information {
    public:

        String timeShort;


        struct System
        {
            uint32_t uptimeSeconds;
            int8_t wifiRSSI;
            String IPAddress;
        };
        struct AudioPlayer
        {
            int channels;
            int bitRate;
            int sampleRate;

            int volume;
        };
        struct Weather
        {
            float temperature;
            double windSpeedKmh;
        };
        


        System system;
        AudioPlayer audioPlayer;
        Weather weather;

};

extern Information information;

void krInfoInitialize(void);

#endif