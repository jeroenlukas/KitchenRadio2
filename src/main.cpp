#include <Arduino.h>

#include <Ticker.h>



#include <WiFi.h>

#include <SPI.h>
#include <LittleFS.h>


#include "TickTwo.h"

#include "logger.h"
#include "main.h"

#include "version.h"

#include "configuration/config.h"
#include "settings/krSettings.h"
#include "information/krInfo.h"
#include "hmi/krFrontpanel.h"
#include "flags.h"
#include "audioplayer/krAudioplayer.h"
#include "webradio/krWebradio.h"
#include "audioplayer/cbuf_ps.h"
#include "webserver/krAsyncWebserver.h"
#include "information/krWeather.h"
#include "information/krTime.h"
#include "configuration/constants.h"
#include "hmi/xbmIcons.h"
#include "hmi/krLamp.h"
#include "hmi/krCLI.h"
#include "hmi/krDisplay.h"
#include "hmi/krBuzzer.h"
#include "audioplayer/krI2S.h"

#include "esp_system.h"
#include "esp_himem.h"
#include "esp_heap_caps.h"

//Via tutorial from 
SPIClass *hspi = NULL;

void ticker_userinput();

Ticker ticker_10ms_ref;
Ticker ticker_100ms_ref;
Ticker ticker_1000ms_ref;
Ticker ticker_1min_ref;
Ticker ticker_30min_ref;
TickTwo ticker_userinput_ref(ticker_userinput, CONF_MENU_RETURN_HOME_MS);


void ticker_10ms()
{
  //front_read_pots();
}

void ticker_100ms()
{
  //front_read_buttons();  
  front_buttons_read();
  display_update_scroll_offset();
}

void ticker_1000ms()
{
  flags.main.passed1000ms = true;
}

void ticker_30min()
{
  flags.main.passed30min = true;
}

void ticker_1min()
{
  flags.main.passed1min = true;
}

void ticker_userinput()
{
  // Return to home menu
  menu = MENU_HOME;
}

void setup() 
{
  Serial.begin(115200);

  delay(100);

  Serial.print("KitchenRadio 2");
  delay(100);  

  cli_init();

  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, HSPI_CS);  

  u8g2.begin(); 
  u8g2.setFont(FONT_BOOTLOG);

  display_set_brightness(100);

  front_init();

  log_boot_begin();

  log_boot("==== KitchenRadio2! ====");
  log_boot("Firmware version: " + String(KR_VERSION));

  delay(100);
  
  // ESP info
  log_boot("Chip model: " + String(ESP.getChipModel()) + " rev " + String(ESP.getChipRevision()));
  log_boot("CPU freq: " + String(ESP.getCpuFreqMHz()) + " MHz");
  log_boot("Total heap: " + String(ESP.getHeapSize()) + " bytes");
  log_boot("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
  log_boot("PSRAM size: " + String(ESP.getPsramSize()) + " bytes");
  delay(300);
    
  // Littlefs
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
      Serial.println("LittleFS Mount Failed");
      while(1);
  }
  else
  {
    log_boot("LittleFS mounted");
  }

   // Should be moved to top
  log_boot("Loading settings");
  //settings_read_config();
  config_read();

  const char * deviceName = settings["devicename"];
  log_boot("Device name: " + String(deviceName));
  const char * location = settings["location"];
  log_boot("Location: " + String(location));

  log_boot("Init buzzer");
  buzzer_init();

  log_boot("Init I2S");
  slavei2s_init();
 
  information.webRadio.station_count = webradio_get_num_stations();
  log_boot("Stations: " + String( information.webRadio.station_count));
  
  // WiFi setup
  log_boot("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  WiFi.setHostname("KitchenRadio2");
  WiFi.disconnect();

  WiFi.begin(CONF_WIFI_SSID, CONF_WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print('.');
      delay(500);
  }

  
  information.system.IPAddress = WiFi.localIP().toString(); 

  log_boot("\nConnected! (" + (information.system.IPAddress) + ")");
  log_boot("RSSI: " + String(WiFi.RSSI()) + " dBm");

  log_boot("Init led ring");
  lamp_init(); 
  
  // Time 
  log_boot("Init time");
  time_init();
  time_waitForSync();
  const char * tz = settings["clock"]["timezone"];
  log_boot("Timezone:" + String(tz));

  // PSRAM buffer
  circBuffer.resize(CIRCBUFFER_SIZE);

  // Codec
  Serial.print("Starting vs1053");
  log_boot("Starting codec...");
  audioplayer_init();

  if(player.isChipConnected())
  {
    log_boot("VS1053 found");
  }
  else
  {
    log_boot("Error: VS1053 not found!");
  }


  audioplayer_set_soundmode(SOUNDMODE_OFF);

  // Set tone control
  audioplayer_setbass(settings["audio"]["tonecontrol"]["bass"]);
  audioplayer_settreble(settings["audio"]["tonecontrol"]["treble"]);

  // Get weather
  log_boot("Retrieve weather");
  weather_retrieve();

  log_boot("Starting webserver");
  webserver_init();

  // Turn off leds
  front_led_off(MCP_LED_WEBRADIO);
  front_led_off(MCP_LED_BLUETOOTH);

  // Tickers
  ticker_10ms_ref.attach(0.01, ticker_10ms);
  ticker_100ms_ref.attach(0.1, ticker_100ms);
  ticker_1000ms_ref.attach(1.0, ticker_1000ms);
  ticker_30min_ref.attach(1.0 * 1800, ticker_30min);
  ticker_1min_ref.attach(1.0 * 60, ticker_1min);  
  ticker_userinput_ref.start();  

  audioplayer_setvolume(50);

  log_boot("Device ready");

  delay(100);

  // Debug log on main screen
  log_debug_init();
}

uint32_t prev_millis = 0;

void loop() 
{


  // TODO change to flag
  if((millis() - prev_millis > 500) || (flags.main.updateLog && (menu == MENU_HOME)))
  {
    //flags.frontPanel.buttonAnyPressed = false;
    prev_millis = millis();
    
    display_draw_menu();
  }
 
  //kcx_read();

  webserver_cleanup_clients();
  
  front_handle();

  webradio_handle_stream();

  slavei2s_handle();

  audioplayer_feedbuffer();  

  cli_handle();

  ticker_userinput_ref.update();

  // -------------------------=== FLAGS ===--------------------------

  // --------------------------- Tickers ----------------------------
  // Execute stuff every second
  if(flags.main.passed1000ms)
  {
    flags.main.passed1000ms = false;

    front_ldr_read();

    display_set_brightness_auto();

    information.system.uptimeSeconds++;
    information.system.wifiRSSI = WiFi.RSSI();
    information.webRadio.buffer_pct = constrain(((double)circBuffer.available() / (double)CONF_AUDIO_MIN_BYTES) * 100, 0, 100);
  }

  // Execute stuff every minute
  if(flags.main.passed1min)
  {
    flags.main.passed1min = false;
    time_update();
  }

  // Execute stuff every 30 minutes
  if(flags.main.passed30min)
  {
    flags.main.passed30min = false;
    log_debug("Weather lookup");
    weather_retrieve();
  }

  // --------------------------- User Events ----------------------------

  if(flags.frontPanel.buttonOffPressed)
  {
      flags.frontPanel.buttonOffPressed = false;

      switch(menu)
      {
        case MENU_HOME:
          audioplayer_set_soundmode(SOUNDMODE_OFF);
          break;
        case MENU_LAMP:
        case MENU_SYSTEM:
        case MENU_ALARM:
          menu = MENU_HOME;
          break;
      }
      
 
  }
  if(flags.frontPanel.buttonRadioPressed)
  {
      flags.frontPanel.buttonRadioPressed = false; 

      switch(menu)
      {
        case MENU_HOME:
          audioplayer_set_soundmode(SOUNDMODE_WEBRADIO);
          break;
        case MENU_LAMP:
          break;
        case MENU_SYSTEM:
          break;
      }
      
  }

  if(flags.frontPanel.buttonBluetoothPressed)
  {
    flags.frontPanel.buttonBluetoothPressed = false;

    switch(menu)
    {
      case MENU_HOME:
        audioplayer_set_soundmode(SOUNDMODE_BLUETOOTH);
        break;
      case MENU_LAMP:
        break;
      case MENU_SYSTEM:
        break;
    }    
  }

  if(flags.frontPanel.buttonSystemPressed)
  {
    flags.frontPanel.buttonSystemPressed = false;
    log_debug("System");
    menu = MENU_SYSTEM;
    menuitem = MITEM_SYSTEM_INFO;
  }
  
  if(flags.frontPanel.buttonAlarmPressed)
  {
    flags.frontPanel.buttonAlarmPressed = false;
    log_debug("Alarm");
  }
  
  if(flags.frontPanel.buttonLampPressed)
  {
    flags.frontPanel.buttonLampPressed = false;
    menu = MENU_LAMP;
    menuitem = MITEM_LAMP_STATE;
  }

  if(flags.frontPanel.buttonLampLongPressed)
  {
    flags.frontPanel.buttonLampLongPressed = false;
    lamp_toggle();
  }

  if (flags.frontPanel.encoder1ButtonPressed)
  {
    flags.frontPanel.encoder1ButtonPressed = false;
    // TODO implement mute function
  }

  if (flags.frontPanel.encoder2ButtonPressed)
  {
    flags.frontPanel.encoder2ButtonPressed = false;

    switch(audioplayer_soundMode)
    {
      case SOUNDMODE_BLUETOOTH:
        slavei2s_playpause();
        break;
    }
  }

  // 'Volume' encoder. For volume and value setting
  if(flags.frontPanel.encoder1TurnLeft)
  {
    flags.frontPanel.encoder1TurnLeft = false;
    switch (menu)
    {
      case MENU_HOME:
        if(information.audioPlayer.volume > 3) audioplayer_setvolume(information.audioPlayer.volume - 3);
        break;
      case MENU_LAMP:
        switch(menuitem)
        {
          case MITEM_LAMP_STATE:
              lamp_toggle();
            break;
          case MITEM_LAMP_HUE:
            lamp_sethue(information.lamp.hue - 0.01);
            break;
          case MITEM_LAMP_SATURATION:
            lamp_setsaturation(information.lamp.saturation - 0.02);
            break;
          case MITEM_LAMP_LIGHTNESS:
            lamp_setlightness(information.lamp.lightness - 0.02);
            break;
          case MITEM_LAMP_EFFECTTYPE:
            if(information.lamp.effect_type > 0)
            {
              lamp_seteffecttype(information.lamp.effect_type - 1); 
            }
            break;
          case MITEM_LAMP_EFFECTSPEED:
            lamp_seteffectspeed(information.lamp.effect_speed - 0.0001);
            break;
        }
        break;

        case MENU_SYSTEM:
          switch(menuitem)
          {
            case MITEM_SYSTEM_BASS:
              audioplayer_setbass(information.audioPlayer.bass - 1);
              break;
            case MITEM_SYSTEM_TREBLE:
              audioplayer_settreble(information.audioPlayer.treble - 1);
              break;
            default:
              break;
          }
          break;
      default:
        break;
    }    
  }



if(flags.frontPanel.encoder1TurnRight)
  {
    flags.frontPanel.encoder1TurnRight = false;
    switch (menu)
    {
      case MENU_HOME:
        if(information.audioPlayer.volume < 100) audioplayer_setvolume(information.audioPlayer.volume + 3);
        break;
      case MENU_LAMP:
        switch(menuitem)
        {
          case MITEM_LAMP_STATE:
            lamp_toggle();
            break;
          case MITEM_LAMP_HUE:
            lamp_sethue(information.lamp.hue + 0.01);
            break;
          case MITEM_LAMP_SATURATION:
            lamp_setsaturation(information.lamp.saturation + 0.02);
            break;
          case MITEM_LAMP_LIGHTNESS:
            lamp_setlightness(information.lamp.lightness + 0.02);
            break;
          case MITEM_LAMP_EFFECTTYPE:
            if(information.lamp.effect_type < LAMP_EFFECT_MAX)
            {
              lamp_seteffecttype(information.lamp.effect_type + 1); 
            }
            break;
          case MITEM_LAMP_EFFECTSPEED:
            lamp_seteffectspeed(information.lamp.effect_speed + 0.0001);
            break;
        }
        break;

        case MENU_SYSTEM:
          switch(menuitem)
          {
            case MITEM_SYSTEM_BASS:
              audioplayer_setbass(information.audioPlayer.bass + 1);
              break;
            case MITEM_SYSTEM_TREBLE:
              audioplayer_settreble(information.audioPlayer.treble + 1);
              break;
            default:
              break;
          }
          break;
      default:
        break;
    }    
  }

  // 'Tune' encoder. For radio station selection and setting selection
  if (flags.frontPanel.encoder2TurnLeft)
  {
    flags.frontPanel.encoder2TurnLeft = false;

    switch(menu)
    {
      case MENU_HOME:
        if(audioplayer_soundMode == SOUNDMODE_WEBRADIO)
        {
          if(information.webRadio.station_index > 0)
          {
            webradio_open_station(information.webRadio.station_index - 1);
          }
        }
        break;
      case MENU_LAMP:
        if(menuitem > MITEM_LAMP_MIN) menuitem--;
        break;
      case MENU_SYSTEM:
        if(menuitem > MITEM_SYSTEM_MIN) menuitem--;
        break;
    }
  }

  if (flags.frontPanel.encoder2TurnRight)
  {
    flags.frontPanel.encoder2TurnRight = false;

    switch(menu)
    {
      case MENU_HOME:
        if(audioplayer_soundMode == SOUNDMODE_WEBRADIO)
        {
          if(information.webRadio.station_index < information.webRadio.station_count - 1)
          {
            webradio_open_station(information.webRadio.station_index + 1);
          }
        }
        break;
      case MENU_LAMP:
        if(menuitem < MITEM_LAMP_MAX) menuitem++;
        break;
      case MENU_SYSTEM:
        if(menuitem < MITEM_SYSTEM_MAX) menuitem++;        
        break;
    }
    
  }

  // Any button was pressed, or encoder turned
  if(flags.frontPanel.buttonAnyPressed)
  {
    flags.frontPanel.buttonAnyPressed = false;
    
    display_draw_menu();

    // Restart the ticker
    ticker_userinput_ref.stop();
    ticker_userinput_ref.start();
  }
}

