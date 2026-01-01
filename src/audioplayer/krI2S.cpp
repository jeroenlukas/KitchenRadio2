#include <Arduino.h>
#include <driver/i2s.h>


#include "configuration/config.h"
#include "configuration/constants.h"
#include "audioplayer/krI2S.h"
#include "audioplayer/krAudioplayer.h"

// Source:
// https://diyi0t.com/i2s-sound-tutorial-for-esp32/
//
// https://dronebotworkshop.com/esp32-i2s/

const i2s_port_t I2S_PORT = I2S_NUM_0;

#define I2S_BUFFERLEN   64
//int16_t sBuffer[I2S_BUFFERLEN];
uint8_t sBuffer[I2S_BUFFERLEN];

unsigned char bt_wav_header[44] = {
    0x52, 0x49, 0x46, 0x46, // RIFF
    0xFF, 0xFF, 0xFF, 0xFF, // size
    0x57, 0x41, 0x56, 0x45, // WAVE
    0x66, 0x6d, 0x74, 0x20, // fmt
    0x10, 0x00, 0x00, 0x00, // subchunk1size
    0x01, 0x00,             // audio format - pcm
    0x02, 0x00,             // numof channels
    0x44, 0xac, 0x00, 0x00, //, //samplerate 44k1: 0x44, 0xac, 0x00, 0x00       48k: 48000: 0x80, 0xbb, 0x00, 0x00,
    0x10, 0xb1, 0x02, 0x00, //byterate
    0x04, 0x00,             // blockalign
    0x10, 0x00,             // bits per sample - 16
    0x64, 0x61, 0x74, 0x61, // subchunk3id -"data"
    0xFF, 0xFF, 0xFF, 0xFF  // subchunk3size (endless)
};

void slavei2s_init()
{
esp_err_t err;

 // The I2S config as per the example
  
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = I2S_BUFFERLEN,
    .use_apll = false
  };
 

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_SLAVEI2S_SCK,   // Serial Clock (SCK)
      .ws_io_num = PIN_SLAVEI2S_WS,    // Word Select (WS)
      .data_out_num = I2S_PIN_NO_CHANGE, // not used (only for speakers)
      .data_in_num = PIN_SLAVEI2S_SD  // Serial Data (SD)
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");

  i2s_start(I2S_PORT);
}

void slavei2s_sendheader()
{
  player.playChunk(bt_wav_header, 44);
}

void slavei2s_handle()
{
    size_t bytesIn = 0;
    if(audioplayer_soundMode == SOUNDMODE_BLUETOOTH)
    {
        esp_err_t result = i2s_read(I2S_PORT, &sBuffer, I2S_BUFFERLEN, &bytesIn, portMAX_DELAY);

        if (result == ESP_OK)
        {
          if(bytesIn > 0)
          {
           // Serial.println("data: " + String(bytesIn));
           // Read I2S data buffer
          /*int16_t samples_read = bytesIn / 8;
          if (samples_read > 0) {
            float mean = 0;
            for (int16_t i = 0; i < samples_read; ++i) {
              mean += (sBuffer[i]);
            }
      
            // Average the data reading
            mean /= samples_read;
      
            // Print to serial plotter
            Serial.println(mean);*/
            if(player.data_request())
            {
              player.playChunk(sBuffer, 64);
            }
          }        
        }
    }
}