#include <Arduino.h>
#include <VS1053.h>

#include "audioplayer/cbuf_ps.h"
#include "audioplayer/krAudioplayer.h"
#include "webradio/krWebradio.h"

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

uint8_t mp3buff[64];

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