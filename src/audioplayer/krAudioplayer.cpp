#include <Arduino.h>
#include <VS1053.h>

#include "audioplayer/cbuf_ps.h"
#include "audioplayer/krAudioplayer.h"
#include "webradio/krWebradio.h"
#include "configuration/constants.h"

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

uint8_t mp3buff[64];

uint8_t audioplayer_soundMode = SOUNDMODE_OFF;

cbuf_ps circBuffer(1024); 

char readBuffer[4096] __attribute__((aligned(4)));

void audioplayer_init()
{
    //u8g2log.print("Starting codec...\n");
    player.begin();
    if(player.isChipConnected())
    {
      //u8g2log.print("VS1053 found\n");
    }
    else
    {
    //  u8g2log.print("ERROR: VS1053 not found\n");
    }
    player.loadDefaultVs1053Patches();
    player.switchToMp3Mode(); // optional, some boards require this
    player.setVolume(80);
}

void audioplayer_set_soundmode(uint8_t soundMode)
{
    switch(audioplayer_soundMode)
    {
        case SOUNDMODE_OFF:
            break;
        case SOUNDMODE_WEBRADIO:
            webradio_stop();
            break;
        case SOUNDMODE_BLUETOOTH:
            // ...           
            break;
       // default:
       //     break;
    }

    switch(soundMode)
    {
        case SOUNDMODE_OFF:

            break;
        case SOUNDMODE_WEBRADIO:
            webradio_open_station(0);
            break;
        case SOUNDMODE_BLUETOOTH:
            uint16_t sci_mode = player.read_register(0x00);
            Serial.println("sci_mode: " + String(sci_mode));
            
            player.writeRegister(0xC, 16000); // aictrl0 samp rate
            player.writeRegister(0xD, 0); // aictrl1, gain
            player.writeRegister(0xE, 4096); // aictrl2 max autogain amp
            player.writeRegister(0xF, 0); // aictrl3 mode

            player.writeRegister(0x0, sci_mode | (1 << 12) | (1 << 14));            
            //player.loadUserCode()

            sci_mode = player.read_register(0x00);
            Serial.println("new sci_mode: " + String(sci_mode));
            break;
    }


    audioplayer_soundMode = soundMode;
}

void IRAM_ATTR audioplayer_feedbuffer()
{
    if(webradio_buffered_enough() == false)
    return;

    if (circBuffer.available() > 0)
    {
        // Does the VS1053 want any more data (yet)?
        if (player.data_request())
        {

            

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
}

void audioplayer_settone(int8_t bass_freq, int8_t bass_gain, int8_t treble_freq, int8_t treble_gain)
{
    uint16_t toneconfig = ((treble_gain << 12) + (treble_freq << 8) + (bass_gain << 4) + bass_freq);
        
    player.writeRegister(0x2, toneconfig);
}

// Flush the buffer, to avoid audio continuing to play once the stream is closed
void audioplayer_flushbuffer()
{
    circBuffer.flush();
}