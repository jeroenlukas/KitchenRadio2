#include <Arduino.h>
#include <VS1053.h>

#include "audioplayer/cbuf_ps.h"
#include "audioplayer/krAudioplayer.h"
#include "webradio/krWebradio.h"
#include "hmi/krFrontpanel.h"
#include "information/krInfo.h"
#include "configuration/constants.h"
#include "settings/krSettings.h"
#include "audioplayer/krI2S.h"
#include "logger.h"
#include "flags.h"

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

uint8_t mp3buff[64];

uint8_t audioplayer_soundMode = SOUNDMODE_OFF;

cbuf_ps circBuffer(1024); 

char readBuffer[4096] __attribute__((aligned(4)));

// Set by system, when audiomode is set to 'off'
void audioplayer_pa_mute(bool mute)
{
    // Toggle power amp mute pin
    digitalWrite(PIN_PA_MUTE, !mute);
}

// Set mute by user
void audioplayer_set_mute(bool mute)
{
    information.audioPlayer.mute = mute;
    audioplayer_pa_mute(mute);
}

void audioplayer_init()
{
    pinMode(PIN_PA_MUTE, OUTPUT);
    
    player.begin();
    
    player.loadDefaultVs1053Patches();
    player.switchToMp3Mode(); // optional, some boards require this
    player.setVolume(80);

    audioplayer_pa_mute(true);
}

void audioplayer_setvolume(uint8_t volume)
{
    volume = constrain(volume, 1, 100);
    information.audioPlayer.volume = volume;
    
    // Convert to logarithmic scale
    uint8_t vol_log = 2.5 * 20 * log10(information.audioPlayer.volume);
    
    player.setVolume(vol_log);
}



void audioplayer_set_soundmode(uint8_t soundMode)
{
    front_led_off(MCP_LED_WEBRADIO);
    front_led_off(MCP_LED_BLUETOOTH);

    // Current soundmode
    switch(audioplayer_soundMode)
    {
        case SOUNDMODE_OFF:
            break;

        case SOUNDMODE_WEBRADIO:
            webradio_stop();
            break;

        case SOUNDMODE_BLUETOOTH:            
            slavei2s_send("AT+END");
            // Clear audio info
            information.audioPlayer.bluetoothArtist = "";
            information.audioPlayer.bluetoothTitle = "";
            break;
    }

    audioplayer_flushbuffer();
    player.softReset();    

    // Switch to the new soundmode
    switch(soundMode)
    {
        case SOUNDMODE_OFF:
            log_debug("Sound off");
            audioplayer_pa_mute(true);
            break;

        case SOUNDMODE_WEBRADIO:
            webradio_open_station(information.webRadio.station_index);
            front_led_on(MCP_LED_WEBRADIO);
            log_debug("Radio mode");
            audioplayer_pa_mute(false);
            break;

        case SOUNDMODE_BLUETOOTH:
           // front_led_on(LED_BLUETOOTH);
            log_debug("Bluetooth mode");
            front_led_on(MCP_LED_BLUETOOTH);

            slavei2s_send("AT+START");
            slavei2s_sendheader();

            audioplayer_pa_mute(false);
            
            break;
    }

    audioplayer_soundMode = soundMode;
   
}

// Send MP3 data  from the circular buffer to the VS1053
// Only used for webradio
void IRAM_ATTR audioplayer_feedbuffer()
{
    if(audioplayer_soundMode != SOUNDMODE_WEBRADIO)
        return;

    if(webradio_buffered_enough() == false)
        return;

    if (circBuffer.available() > 0)
    {
        // Does the VS1053 want any more data (yet)?
        if (player.data_request())
        {           
            int bytesRead=0;

            bytesRead = circBuffer.read((char *)mp3buff, 32);

            
            // If we didn't read the full 32 bytes, that's a worry
            if (bytesRead < 32)
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
    /*
    ST_AMPLITUDE 15:12  Treble Control in 1.5 dB steps (-8..7, 0 = off)
    ST_FREQLIMIT 11:8   Lower limit frequency in 1000 Hz steps (1..15)
    SB_AMPLITUDE 7:4    Bass Enhancement in 1 dB steps (0..15, 0 = off)
    SB_FREQLIMIT 3:0    Lower limit frequency in 10 Hz steps (2..15
    */
    uint16_t toneconfig = ((treble_gain << 12) + (treble_freq << 8) + (bass_gain << 4) + bass_freq);
        
    player.writeRegister(0x2, toneconfig);
}

void audioplayer_setbass(int8_t bass_gain)
{
    int gain = constrain(bass_gain, 0, +15);
    information.audioPlayer.bass = gain;

    settings["audio"]["tonecontrol"]["bass"] = gain;
    
    // Change the tonecontrol
    audioplayer_settone(
        int(settings["audio"]["tonecontrol"]["bass_freq"]),
        information.audioPlayer.bass,
        int(settings["audio"]["tonecontrol"]["treble_freq"]),
        information.audioPlayer.treble
        );
    

}

void audioplayer_settreble(int8_t treble_gain)
{
    int gain = constrain(treble_gain, -8, +7);
    information.audioPlayer.treble = gain;

    settings["audio"]["tonecontrol"]["treble"] = gain;

    // Change the tonecontrol
    audioplayer_settone(
        int(settings["audio"]["tonecontrol"]["bass_freq"]),
        information.audioPlayer.bass,
        int(settings["audio"]["tonecontrol"]["treble_freq"]),
        information.audioPlayer.treble
        );
}

// Flush the buffer, to avoid audio continuing to play once the stream is closed
void audioplayer_flushbuffer()
{
    circBuffer.flush();
}