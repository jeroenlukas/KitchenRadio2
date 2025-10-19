#include <Arduino.h>

#include <Ticker.h>

#include <U8g2lib.h>

#include <WiFi.h>

#include <SPI.h>
#include <LittleFS.h>




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
#include "audioplayer/kcx.h"
//#include "hmi/krMonitor.h"
#include "hmi/xbmIcons.h"
#include "hmi/krLamp.h"
#include "hmi/krCLI.h"

#include "esp_system.h"
#include "esp_himem.h"
#include "esp_heap_caps.h"

//Via tutorial from 
SPIClass *hspi = NULL;

Ticker ticker_10ms_ref;
Ticker ticker_100ms_ref;
Ticker ticker_1000ms_ref;
Ticker ticker_1min_ref;
Ticker ticker_30min_ref;

U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ HSPI_CS, /* dc=*/ HSPI_DC, /* reset=*/ 9);	// Enable U8G2_16BIT in u8g2.h

void ticker_10ms()
{
  front_read_pots();
}

void ticker_100ms()
{
  front_read_buttons();  
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

void setup() 
{
  Serial.begin(115200);
  Serial.print("KitchenRadio 2");

  cli_init();

  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, HSPI_CS);

  front_init();

  u8g2.begin(); 
  u8g2.setFont(FONT_BOOTLOG);

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


  kcx_init();

  audioplayer_set_soundmode(SOUNDMODE_OFF);

  

 
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

  log_boot("Init led ring");
  lamp_init();

  //log_boot(localTimezone.dateTime("D d M"));

  // Should be moved to top
  log_boot("Loading settings");
  settings_read_config();
  const char * deviceName = settings["deviceName"];
  log_boot("Device name: " + String(deviceName));
  const char * location = settings["location"];
  log_boot("Location: " + String(location));
  
  // Time 
  time_init();
  time_waitForSync();
  const char * tz = settings["clock"]["timezone"];
  log_boot("Timezone:" + String(tz));

  

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
  ticker_10ms_ref.attach(0.01, ticker_10ms);
  ticker_100ms_ref.attach(0.1, ticker_100ms);
  ticker_1000ms_ref.attach(1.0, ticker_1000ms);
  ticker_30min_ref.attach(1.0 * 1800, ticker_30min);
  ticker_1min_ref.attach(1.0 * 60, ticker_1min);

  flags.frontPanel.volumePotChanged = true;

  // Debug log on main screen
  log_debug_init();
}

uint32_t prev_millis = 0;

void loop() 
{


// TODO change to flag
  if((millis() - prev_millis > 500) || (flags.main.updateLog && (menu == MENU_HOME)))
  {
    prev_millis = millis();
    
    u8g2.firstPage();

    // This draws the main screen. Only screen related stuff should be done here.
    do {

      switch(menu)
      {
        case MENU_HOME:
          // Weather
          if(settings["homedisplay"] == "debug")
          {
            u8g2.setFont(FONT_S);
            u8g2.drawStr(3, 6, (information.weather.stateShort).c_str());
            u8g2.drawStr(3, 14, ("Temp: " + String(information.weather.temperature) + "\xb0" + "C").c_str());
            u8g2.drawStr(3, 22, ("Wind: " + String(information.weather.windSpeedKmh) + " kmh").c_str());
            u8g2.drawStr(3, 30, ("RSSI: " + String(information.system.wifiRSSI) + " dBm").c_str());
            u8g2.drawStr(3, 38, ("Buf: " + String(circBuffer.available()) + " B").c_str());
          }
          else if(settings["homedisplay"] == "normal")
          {
            u8g2.setFont(u8g2_font_open_iconic_weather_4x_t);
            int weatherglyph = 0;
            // https://openweathermap.org/weather-conditions
            if(information.weather.stateCode <= 232) weatherglyph = 64+3; // thunderstorm
            if(information.weather.stateCode <= 321) weatherglyph = 64+3; // drizzle
            if(information.weather.stateCode <= 531) weatherglyph = 64+3; // raim
            if(information.weather.stateCode <= 622) weatherglyph = 64+3; // snow
            if(information.weather.stateCode <= 781) weatherglyph = 64+4; // atmosphere
            if(information.weather.stateCode <= 800) weatherglyph = 64+5; // clear
            if(information.weather.stateCode <= 804) weatherglyph = 64+0; // clouds
            u8g2.drawGlyph(5, 35, weatherglyph);
            u8g2.setFont(FONT_M);
            u8g2.drawStr(42, 10,(String(information.weather.temperature,1) + " C").c_str());
            u8g2.drawStr(42, 20,(String(information.weather.windSpeedKmh,0) + " kmh").c_str());
            u8g2.drawStr(42, 30,(String(information.weather.stateShort)).c_str());
            u8g2.setFont(FONT_S);
            u8g2.drawStr(POSX_CLOCK + 30, 60, ("B: " + String(circBuffer.available()) + " B").c_str());
          }
             
          

          // Clock
          u8g2.setFont(FONT_CLOCK);
          u8g2.setCursor(POSX_CLOCK, POSY_CLOCK);
          u8g2.print(u8x8_u8toa(information.hour, 2)); 
          u8g2.drawStr(POSX_CLOCK + 30, POSY_CLOCK - 2, ":");
          u8g2.setCursor(POSX_CLOCK + 39, POSY_CLOCK);
          u8g2.print(u8x8_u8toa(information.minute, 2));
          
          // Date
          u8g2.setFont(FONT_S);
          u8g2.drawStr(POSX_CLOCK + 10, POSY_CLOCK + 12, (information.dateMid).c_str());

          // Misc
          u8g2.setFont(FONT_S);
          //u8g2.drawStr(POSX_CLOCK + 30, 60, ("LDR: " + String(information.system.ldr) + "%").c_str());

          u8g2.drawLine(0, 44, 256, 44);

          // Sound mode / Station 
          
          switch(audioplayer_soundMode)
          {
            case SOUNDMODE_OFF:
            
              u8g2.setFont(u8g2_font_lastapprenticebold_tr);
              u8g2.drawStr(POSX_AUDIO, POSY_AUDIO, "(Off)");
              break;
            case SOUNDMODE_WEBRADIO:
              u8g2.drawXBM(POSX_AUDIO_ICON, POSY_AUDIO_ICON-16, xbm_radio_width, xbm_radio_height, xbm_radio_bits);
              u8g2.setFont(u8g2_font_lastapprenticebold_tr);
              u8g2.drawStr(POSX_AUDIO, POSY_AUDIO, (String(information.webRadio.stationIndex + 1) + "/" + String(information.webRadio.stationCount) + " " + (information.webRadio.stationName)).c_str());
              break;
            case SOUNDMODE_BLUETOOTH:
              u8g2.drawXBM(POSX_AUDIO_ICON, POSY_AUDIO_ICON-16, xbm_bluetooth_width, xbm_bluetooth_height, xbm_bluetooth_bits);

              u8g2.setFont(u8g2_font_lastapprenticebold_tr);
              u8g2.drawStr(POSX_AUDIO, POSY_AUDIO, "Bluetooth");
              u8g2.setFont(u8g2_font_open_iconic_all_2x_t);
              switch(information.audioPlayer.bluetoothMode)
              {
                case KCX_OFF:
                  //u8g2.drawStr(POSX_AUDIO + 80, POSY_AUDIO, "Off");
                  u8g2.drawGlyph(POSX_AUDIO + 80, POSY_AUDIO, 285);
                  break;
                case KCX_NOTCONNECTED:
                  u8g2.drawGlyph(POSX_AUDIO + 80, POSY_AUDIO, 285);
                  break;
                case KCX_PAUSED:
                  u8g2.drawGlyph(POSX_AUDIO + 80, POSY_AUDIO, 210);
                  break;
                case KCX_PLAYING:
                  u8g2.drawGlyph(POSX_AUDIO + 80, POSY_AUDIO, 211);
                  break;
                case KCX_UNKNOWN:            
                  u8g2.drawStr(POSX_AUDIO + 80, POSY_AUDIO, "(?)");
                  break;
              }
              break;
          }

          // Log window
          log_debug_draw();
          
          break; // END MENU_HOME

        case MENU_LAMP:
          u8g2.setFont(u8g2_font_lastapprenticebold_tr);
          u8g2.drawStr(10, 10, "Lamp");
          u8g2.setFont(FONT_S);
          u8g2.drawStr(2, 62, "Back                 up                  down");

          
          switch(menuitem)
          {
            case MITEM_LAMP_STATE:
              u8g2.drawStr(10, 30, "State:");
              u8g2.drawStr(80, 30, mitem_lamp_state_desc[information.lamp.state].c_str());
              break;
            case MITEM_LAMP_HUE:
              u8g2.drawStr(10, 30, "Hue:");
              u8g2.drawStr(80, 30, String(information.lamp.hue).c_str());
              break;
            case MITEM_LAMP_EFFECTTYPE:
              u8g2.drawStr(10, 30, "Effect type:");
              //u8g2.drawStr(80, 30, String(information.lamp.effect_type).c_str());
              u8g2.drawStr(80, 30, mitem_lamp_effecttype_desc[information.lamp.effect_type].c_str());
              break;
            case MITEM_LAMP_EFFECTSPEED:
              u8g2.drawStr(10, 30, "Effect speed:");
              u8g2.drawStr(80, 30, String((int)(information.lamp.effect_speed*1000)).c_str());
              break;
          }
          break;

          

      }

      

    } while ( u8g2.nextPage() );


  }


  
 
  kcx_read();

  webserver_cleanup_clients();
  
  front_multibuttons_loop();

  webradio_handle_stream();

  audioplayer_feedbuffer();

  front_read_encoder();

  //mon_receiveCommand();
  if (Serial.available()) 
  {
    static char commandbuffer[100] = "";
    static uint8_t commandbuffer_idx = 0;

    char inp = Serial.read();
    Serial.print('\r');
    //Serial.print((char)inp);
    
    commandbuffer[commandbuffer_idx] = inp;
    commandbuffer_idx ++;
    commandbuffer[commandbuffer_idx] = '\0';

    Serial.print(commandbuffer);

    if(inp == '\n')
    {
      
      String commandstr = String(commandbuffer);
      cli_parse(commandstr);
      commandbuffer_idx = 0;
    }

    //char buf[10];
    //String inp = Serial.read(buf, Serial.available())
        // Read out string from the serial monitor
        /*String input = Serial.readStringUntil('\n');

        // Echo the user input
        Serial.print("# ");
        Serial.println(input);

        // Parse the user input into the CLI
        cli_parse(input);*/
    }


  // -------------------------=== FLAGS ===--------------------------

  // --------------------------- Tickers ----------------------------
  // Execute stuff every second
  if(flags.main.passed1000ms)
  {
    flags.main.passed1000ms = false;

    front_read_ldr();

    kcx_getstatus();

    information.system.uptimeSeconds++;
    information.system.wifiRSSI = WiFi.RSSI();
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

  // --------------------------- Events ----------------------------

  if (flags.frontPanel.volumePotChanged)
  {
    flags.frontPanel.volumePotChanged = false;

    if(audioplayer_soundMode == SOUNDMODE_WEBRADIO)
    {
      player.setVolume(log(information.audioPlayer.volume + 1) / log(127) * 100);
      Serial.println(information.audioPlayer.volume);
    }
    else if(audioplayer_soundMode == SOUNDMODE_BLUETOOTH)
    {
      uint16_t rec_gain = 1; // 1024 = 1.0x digital gain! - checken of setVolume() ook werkt bij recording mode.
      if(information.audioPlayer.volume == 0)
      {
        rec_gain = 1;
      }

      else rec_gain = information.audioPlayer.volume * 10;
      player.setVolume(100);
      player.writeRegister(0xD, rec_gain); // recording gian
      Serial.println("rec_gain: " + String(rec_gain));
    }

    log_debug("Volume: " + String(information.audioPlayer.volume));
  }

  if(flags.frontPanel.buttonOffPressed)
  {
      flags.frontPanel.buttonOffPressed = false;

      switch(menu)
      {
        case MENU_HOME:
          audioplayer_set_soundmode(SOUNDMODE_OFF);
          break;
        case MENU_LAMP:
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
          // up
          if(menuitem > MITEM_LAMP_STATE) menuitem--;
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
        // down
        if(menuitem < MITEM_LAMP_EFFECTSPEED) menuitem++;
        break;
    }
    
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
    menu = MENU_LAMP;
    menuitem = MITEM_LAMP_STATE;
    lamp_toggle();

  }

  if (flags.frontPanel.encoderButtonPressed)
  {
    flags.frontPanel.encoderButtonPressed = false;

    switch(audioplayer_soundMode)
    {
      case SOUNDMODE_BLUETOOTH:
        kcx_pausePlay();
        break;
    }
  }

  if (flags.frontPanel.encoderTurnLeft)
  {
    flags.frontPanel.encoderTurnLeft = false;

    switch(menu)
    {
      case MENU_HOME:
        if(audioplayer_soundMode == SOUNDMODE_WEBRADIO)
        {
          if(webradio_stationIndex > 0)
          {
            webradio_stationIndex--;
            webradio_open_station(webradio_stationIndex);
          }      
        }
        break;
      case MENU_LAMP:
        switch(menuitem)
        {
          case MITEM_LAMP_STATE:
            if(information.lamp.state)
              lamp_toggle();
            break;
          case MITEM_LAMP_HUE:
            lamp_sethue(information.lamp.hue - 0.05);
            break;
          case MITEM_LAMP_EFFECTTYPE:
            if(information.lamp.effect_type > 0)
            {
              lamp_seteffecttype(information.lamp.effect_type - 1); 
            }
            break;
          case MITEM_LAMP_EFFECTSPEED:
            lamp_seteffectspeed(information.lamp.effect_speed - 0.001);
            break;
        }
        break;
    }
  }

  if (flags.frontPanel.encoderTurnRight)
  {
    flags.frontPanel.encoderTurnRight = false;

    switch(menu)
    {
      case MENU_HOME:
        if(audioplayer_soundMode == SOUNDMODE_WEBRADIO)
        {
          if(webradio_stationIndex < information.webRadio.stationCount - 1)
          {
            webradio_stationIndex++;      
            webradio_open_station(webradio_stationIndex);
          }
        }
        break;
      case MENU_LAMP:
        switch(menuitem)
        {
          case MITEM_LAMP_STATE:
            if(!information.lamp.state)
              lamp_toggle();
            break;
          case MITEM_LAMP_HUE:
            lamp_sethue(information.lamp.hue + 0.05);
            break;
          case MITEM_LAMP_EFFECTTYPE:
            if(information.lamp.effect_type < LAMP_EFFECT_MAX)
            {
              lamp_seteffecttype(information.lamp.effect_type + 1);
            }   
            break;
          case MITEM_LAMP_EFFECTSPEED:
            lamp_seteffectspeed(information.lamp.effect_speed + 0.001);
            break;
        }
        break;
    }
    
  }
}

