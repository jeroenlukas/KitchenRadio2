#include <Arduino.h>

#include <Ticker.h>

#include <U8g2lib.h>

#include <WiFi.h>

#include <SPI.h>

// Temp
#include <HardwareSerial.h>

#include "logger.h"
#include "main.h"

#include "version.h"

#include "configuration/config.h"
#include "information/krInfo.h"
#include "hmi/krFrontpanel.h"
#include "flags.h"
#include "audioplayer/krAudioplayer.h"
#include "webradio/krWebradio.h"
#include "audioplayer/cbuf_ps.h"

#include "esp_system.h"
#include "esp_himem.h"
#include "esp_heap_caps.h"

//Via tutorial from 
SPIClass *hspi = NULL;

//WiFiClient client;

HardwareSerial serialKcx(2);

Ticker ticker_100ms_ref;
Ticker ticker_1000ms_ref;

U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ HSPI_CS, /* dc=*/ HSPI_DC, /* reset=*/ 9);	// Enable U8G2_16BIT in u8g2.h




 char *host = "icecast.omroep.nl";
     char *path = "/radio1-bb-mp3";
    int httpPort = 80;/// 8563;

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

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial.print("hoiix");

  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, HSPI_CS);

  frontpanel_setup();

  u8g2.begin(); 
  u8g2.setFont(U8LOG_FONT);

  log_boot_begin();




  serialKcx.begin(9600, SERIAL_8N1, KCX_RX, KCX_TX);

  log_boot("     -== KitchenRadio2! ==-");

  // Version
  log_boot("Firmware version: " + String(KR_VERSION));
 
  delay(300);
  // ESP info
  log_boot("Total heap: " + String(ESP.getHeapSize()) + " bytes");
  log_boot("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
  log_boot("PSRAM size: " + String(ESP.getPsramSize()) + " bytes");
  delay(300);
  log_boot("CPU freq: " + String(ESP.getCpuFreqMHz()) + " MHz");
  log_boot("Chip model: " + String(ESP.getChipModel()) + " rev " + String(ESP.getChipRevision()));
  delay(300);

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

  delay(500);

  // Tickers
  ticker_100ms_ref.attach(0.1, ticker_100ms);
  ticker_1000ms_ref.attach(1.0, ticker_1000ms);


  // Debug log on main screen
  log_debug_init();
}

uint8_t hour;
uint8_t secs;

#define POS_CLOCK 210

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

      // Clock
      u8g2.setFont(FONT_S);
      u8g2.drawStr(3, 24, ("IP: " + information.system.IPAddress).c_str());
      u8g2.drawStr(3, 34, ("RSSI: " + String(WiFi.RSSI()) + " dBm").c_str());
      u8g2.drawStr(3, 44, ("Buf: " + String(circBuffer.available()) + " B").c_str());

      u8g2.setFont(u8g2_font_lastapprenticebold_tr);
      u8g2.setCursor(POS_CLOCK + 4,14);
      u8g2.print(u8x8_u8toa(hour,2)); //u8x8_u8toa(cnt,3));
      u8g2.drawStr(POS_CLOCK + 22, 12, ":");
      u8g2.setCursor(POS_CLOCK + 29, 14);
      u8g2.print(u8x8_u8toa(secs,2));


      u8g2.setFont(u8g2_font_lastapprenticebold_tr);
      u8g2.drawStr(2, 62, "NPO Radio 7");

      // Log window
      u8g2.setFont(U8LOG_FONT);    

      // Add a log line if needed      
      if(flags.main.updateLog)
      {
        flags.main.updateLog = false;
        log_debug_print();
      }

      // Draw the log window
      log_debug_draw();

    } while ( u8g2.nextPage() );


  }


  // Execute stuff every second
  if(flags.main.passed1000ms)
  {
    flags.main.passed1000ms = false;

    secs++;
    
    if(secs > 59)
    {
      secs = 0;
      hour++;
    }
    if(hour > 23)
    {
      hour = 0;
    }
  }
 

  
  front_multibuttons_loop();

  webradio_handle_stream();

  audioplayer_feedbuffer();

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
      webradio_stop();
      front_led_off(LED_WEBRADIO);
      front_led_off(LED_BLUETOOTH);
  }
  if(flags.frontPanel.buttonRadioPressed)
  {
      flags.frontPanel.buttonRadioPressed = false;
      Serial.println("radio");
      log_debug("Radio");
      //set_sound_mode(SOUNDMODE_WEBRADIO);

      webradio_open_url(host, path);

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
  }

  if(flags.frontPanel.buttonSystemPressed)
  {
    flags.frontPanel.buttonSystemPressed = false;
    log_debug("System");
  }
  
  if(flags.frontPanel.buttonAlarmPressed)
  {
    flags.frontPanel.buttonAlarmPressed = false;
    log_debug("Alarm");
  }
  
  if(flags.frontPanel.buttonLampPressed)
  {
    flags.frontPanel.buttonLampPressed = false;
    log_debug("Lamp");
  }


}

