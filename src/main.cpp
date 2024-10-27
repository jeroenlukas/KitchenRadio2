#include <Arduino.h>

#include <Ticker.h>

#include <U8g2lib.h>

#include <WiFi.h>

#include <SPI.h>
#include <LittleFS.h>

// Temp
#include <HardwareSerial.h>

#include <ezTime.h>

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
#include "configuration/constants.h"

#include "esp_system.h"
#include "esp_himem.h"
#include "esp_heap_caps.h"

//Via tutorial from 
SPIClass *hspi = NULL;

//WiFiClient client;

HardwareSerial serialKcx(2);

Ticker ticker_100ms_ref;
Ticker ticker_1000ms_ref;
Ticker ticker_30min_ref;

U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ HSPI_CS, /* dc=*/ HSPI_DC, /* reset=*/ 9);	// Enable U8G2_16BIT in u8g2.h

Timezone localTimezone;


 //char *host = "icecast.omroep.nl";
   //  char *path = "/radio1-bb-mp3";
    //int httpPort = 80;/// 8563;

void ticker_100ms()
{
    //webserver_handleclient();
    // Read pots
    front_read_buttons();
    front_read_pots();
}

void ticker_1000ms()
{
  flags.main.passed1000ms = true;
}

void ticker_30min()
{
  flags.main.passed30min = true;
}

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial.print("KitchenRadio 2");

  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, HSPI_CS);

  frontpanel_setup();

  u8g2.begin(); 
  u8g2.setFont(U8LOG_FONT);

  u8g2.setContrast(1);

  log_boot_begin();

  log_boot("==== KitchenRadio2! ====");
    // Version
  log_boot("Firmware version: " + String(KR_VERSION));

  // Littlefs
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
      Serial.println("LittleFS Mount Failed");
      return;
  }
  else
  {
    log_boot("LittleFS mounted");
  }


  audioplayer_set_soundmode(SOUNDMODE_OFF);

  // Bluetooth module
  serialKcx.begin(9600, SERIAL_8N1, KCX_RX, KCX_TX);

  


 
  delay(300);
  // ESP info
  log_boot("Total heap: " + String(ESP.getHeapSize()) + " bytes");
  log_boot("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
  log_boot("PSRAM size: " + String(ESP.getPsramSize()) + " bytes");
  delay(300);
  log_boot("CPU freq: " + String(ESP.getCpuFreqMHz()) + " MHz");
  log_boot("Chip model: " + String(ESP.getChipModel()) + " rev " + String(ESP.getChipRevision()));
  delay(300);


  information.webRadio.stationCount = webradio_get_num_stations();
  log_boot("Stations: " + String( information.webRadio.stationCount));
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
    //  u8g2log.print(".");
      Serial.print('.');
      delay(500);
  }

  
  information.system.IPAddress = WiFi.localIP().toString(); 

  /*u8g2log.setRedrawMode(0);*/
  log_boot("\nConnected! (" + (information.system.IPAddress) + ")");
  log_boot("RSSI: " + String(WiFi.RSSI()) + " dBm");

  

  log_boot(localTimezone.dateTime("D d M"));

  // Should be moved to top
  log_boot("Loading settings");
  settings_read_config();
  const char * deviceName = settings["deviceName"];
  log_boot("Device name: " + String(deviceName));
  const char * location = settings["location"];
  log_boot("Location: " + String(location));
  
  // Time - move to krTime!
  waitForSync();
  const char * timeZone = settings["clock"]["timezone"];
  localTimezone.setLocation(timeZone);
  log_boot("Timezone:" + String(timeZone));



  delay(300);

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

  // Set tone control
  audioplayer_settone(int(settings["audio"]["toneControl"]["bassFreq"]), 
                      int(settings["audio"]["toneControl"]["bassGain"]), 
                      int(settings["audio"]["toneControl"]["trebleFreq"]), 
                      int(settings["audio"]["toneControl"]["trebleGain"]));


  // Get weather
  weather_retrieve();

  log_boot("Starting webserver");
  webserver_init();

  delay(1000);

  


  // Tickers
  ticker_100ms_ref.attach(0.1, ticker_100ms);
  ticker_1000ms_ref.attach(1.0, ticker_1000ms);
  ticker_30min_ref.attach(1.0 * 1800, ticker_30min);


  // Debug log on main screen
  log_debug_init();
}

//uint8_t hour;
//uint8_t secs;

//uint8_t stationIndex = 0;

uint32_t prev_millis = 0;

void loop() 
{


// TODO change to flag
  if((millis() - prev_millis > 1000) || (flags.main.updateLog))
  {
    prev_millis = millis();
    
    u8g2.firstPage();

    // This draws the main screen. Only screen related stuff should be done here.
    do {

      // Weather
      u8g2.setFont(FONT_S);
      u8g2.drawStr(3, 8, (information.weather.stateShort).c_str());
      u8g2.drawStr(3, 17, ("Temp: " + String(information.weather.temperature) + "\xb0" + "C").c_str());
      u8g2.drawStr(3, 26, ("Wind: " + String(information.weather.windSpeedKmh) + " kmh").c_str());
      u8g2.drawStr(3, 35, ("RSSI: " + String(information.system.wifiRSSI) + " dBm").c_str());
      u8g2.drawStr(3, 44, ("Buf: " + String(circBuffer.available()) + " B").c_str());

      // Clock
      u8g2.setFont(FONT_CLOCK);
      u8g2.setCursor(POSX_CLOCK, POSY_CLOCK);
      u8g2.print(u8x8_u8toa(localTimezone.hour(),2)); 
      u8g2.drawStr(POSX_CLOCK + 30, POSY_CLOCK - 2, ":");
      u8g2.setCursor(POSX_CLOCK + 39, POSY_CLOCK);
      u8g2.print(u8x8_u8toa(localTimezone.minute(),2));
      
      // Date
      u8g2.setFont(FONT_S);
      u8g2.drawStr(POSX_CLOCK + 10, POSY_CLOCK + 12, (localTimezone.dateTime("D d M")).c_str());


      // Sound mode / Station 
      u8g2.setFont(u8g2_font_lastapprenticebold_tr);
      switch(audioplayer_soundMode)
      {
        case SOUNDMODE_OFF:
          u8g2.drawStr(2, 62, "(Off)");
          break;
        case SOUNDMODE_WEBRADIO:
          u8g2.drawStr(2, 62, (information.webRadio.stationName).c_str());
          break;
        case SOUNDMODE_BLUETOOTH:
          u8g2.drawStr(2, 62, "Bluetooth");
          break;
      }

      // Log window
      log_debug_draw();



    } while ( u8g2.nextPage() );


  }


  // Execute stuff every second
  if(flags.main.passed1000ms)
  {
    flags.main.passed1000ms = false;

    webserver_notify_clients();

    information.system.uptimeSeconds++;
    information.system.wifiRSSI = WiFi.RSSI();
  }

  // Execute stuff every 30 minutes
  if(flags.main.passed30min)
  {
    flags.main.passed30min = false;
    log_debug("Weather lookup");
    weather_retrieve();
  }
 
  webserver_cleanup_clients();
  
  front_multibuttons_loop();

  webradio_handle_stream();

  audioplayer_feedbuffer();

  //front_read_buttons();

  front_read_encoder();

  // -------------------------=== FLAGS ===--------------------------

  if (flags.frontPanel.volumePotChanged)
  {
    flags.frontPanel.volumePotChanged = false;
    player.setVolume(log(front_pot_vol + 1) / log(127) * 100);
    Serial.println(front_pot_vol);
    
  //  printLogLine("Volume: " + String(front_pot_vol));
    log_debug("Volume: " + String(front_pot_vol));
  }

  if(flags.frontPanel.buttonOffPressed)
  {
      flags.frontPanel.buttonOffPressed = false;
      //set_sound_mode(SOUNDMODE_OFF);
      Serial.println("OFF");
      log_debug("Sound off");
      //webradio_stop();

      audioplayer_set_soundmode(SOUNDMODE_OFF);
      front_led_off(LED_WEBRADIO);
      front_led_off(LED_BLUETOOTH);
  }
  if(flags.frontPanel.buttonRadioPressed)
  {
      flags.frontPanel.buttonRadioPressed = false;
      Serial.println("radio");
      log_debug("Radio");
      //set_sound_mode(SOUNDMODE_WEBRADIO);

      //webradio_open_url(host, path);
      audioplayer_set_soundmode(SOUNDMODE_WEBRADIO);

      front_led_on(LED_WEBRADIO);
      front_led_off(LED_BLUETOOTH);
  }

  if(flags.frontPanel.buttonBluetoothPressed)
  {
    flags.frontPanel.buttonBluetoothPressed = false;
    front_led_off(LED_WEBRADIO);
    front_led_on(LED_BLUETOOTH);
    log_debug("Bluetooth");
    Serial.println("bluetooth");
    audioplayer_set_soundmode(SOUNDMODE_BLUETOOTH);
  }

  if(flags.frontPanel.buttonSystemPressed)
  {
    flags.frontPanel.buttonSystemPressed = false;
    log_debug("System");
    audioplayer_settone(2,0,15,0); // flat
  }
  
  if(flags.frontPanel.buttonAlarmPressed)
  {
    flags.frontPanel.buttonAlarmPressed = false;
    log_debug("Alarm");
    audioplayer_settone(15,15,3,-6); // bass boost treble cut
  }
  
  if(flags.frontPanel.buttonLampPressed)
  {
    flags.frontPanel.buttonLampPressed = false;
    log_debug("Lamp");
  }

  if (flags.frontPanel.encoderButtonPressed)
  {
    flags.frontPanel.encoderButtonPressed = false;
  }

  if (flags.frontPanel.encoderTurnLeft)
  {
    flags.frontPanel.encoderTurnLeft = false;

    if(audioplayer_soundMode == SOUNDMODE_WEBRADIO)
    {
      if(webradio_stationIndex > 0)
      {
        webradio_stationIndex--;
        webradio_open_station(webradio_stationIndex);
      }      
    }
  }

  if (flags.frontPanel.encoderTurnRight)
  {
    flags.frontPanel.encoderTurnRight = false;

    if(audioplayer_soundMode == SOUNDMODE_WEBRADIO)
    {
      if(webradio_stationIndex < information.webRadio.stationCount - 1)
      {
        webradio_stationIndex++;      
        webradio_open_station(webradio_stationIndex);
      }
    }
  }
}

