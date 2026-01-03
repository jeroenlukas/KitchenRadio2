#include <Arduino.h>
#include <driver/i2s.h>


#include "configuration/config.h"
#include "configuration/configPinout.h"
#include "configuration/constants.h"
#include "audioplayer/krI2S.h"
#include "audioplayer/krAudioplayer.h"
#include "information/krInfo.h"


// Source:
// https://diyi0t.com/i2s-sound-tutorial-for-esp32/
//
// https://dronebotworkshop.com/esp32-i2s/

HardwareSerial serial_bt(2);

const i2s_port_t I2S_PORT = I2S_NUM_0;


//#define MIN_BUFFER_FILL 512
#define I2S_BUFFERLEN   64
uint8_t sBuffer[I2S_BUFFERLEN];
//char sBuffer[I2S_BUFFERLEN];

uint8_t bt_wav_header[44] = {
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
  // Init I2S
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
    .dma_buf_len = 512, //I2S_BUFFERLEN,
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

  // Init UART
  serial_bt.begin(115200, SERIAL_8N1, PIN_UART_BT_RX, PIN_UART_BT_TX);
  slavei2s_send("AT+END");
}

// Send WAV header to notify VS1053
void slavei2s_sendheader()
{
  player.playChunk(bt_wav_header, 44);
  serial_bt.println("header!!!");
}

void slavei2s_command_parse(String command)
{
  if(command == "AT+AUDIOSTATE=PLAYING")
  {
    Serial.println("PLaying");
    information.audioPlayer.bluetoothMode = KCX_PLAYING;
  }
  else if(command == "AT+AUDIOSTATE=PAUSED")
  {
    Serial.println("Paused");
    information.audioPlayer.bluetoothMode = KCX_PAUSED;
  }
  else if(command == "AT+AUDIOSTATE=STOPPED")
  {
    Serial.println("Stopped");
    information.audioPlayer.bluetoothMode = KCX_STOPPED;
  }
  else if(command.startsWith("AT+TITLE"))
  {
    information.audioPlayer.bluetoothTitle = command.substring(9);
  }
  else if(command.startsWith("AT+ARTIST"))
  {
    information.audioPlayer.bluetoothArtist = command.substring(10);
  }
}

// Read data from I2S DMA buffer and send it to the VS1053.
// Also parse UART commands
void slavei2s_handle()
{
  size_t bytesIn = 0;
  if(audioplayer_soundMode == SOUNDMODE_BLUETOOTH)
  {
    esp_err_t result = i2s_read(I2S_PORT, &sBuffer, I2S_BUFFERLEN, &bytesIn, 10);// portMAX_DELAY);

    if ((result == ESP_OK) && (bytesIn > 0) && player.data_request())
    {          
      player.playChunk(sBuffer, bytesIn);          
    }
  }

  if(serial_bt.available() > 0)
  {
    String str = serial_bt.readStringUntil('\n');      
    
    Serial.println("Recv: "  + str);
    slavei2s_command_parse(str);
  }
}

void slavei2s_send(String str)
{  
  serial_bt.print(str + '\n');
}