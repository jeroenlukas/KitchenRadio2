#include <Arduino.h>

#include <Ticker.h>

#include <U8g2lib.h>

#include <WiFi.h>

#include <SPI.h>

// Temp
#include <HardwareSerial.h>

#include "log.h"

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


uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];



String logline;
bool updateLog = false;

void printLogLine(String line)
{
  logline = line + "\n";
  updateLog = true;
}

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
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8g2log.setRedrawMode(0);


  serialKcx.begin(9600, SERIAL_8N1, KCX_RX, KCX_TX);

  u8g2log.print("-== KitchenRadio2! ==-\n");

  // Version
  u8g2log.print("Firmware version: " + String(KR_VERSION) + "\n");
 
  

  delay(300);
  // ESP info
  u8g2log.print("Total heap: " + String(ESP.getHeapSize()) + " bytes\n");
  u8g2log.print("Free heap: " + String(ESP.getFreeHeap()) + " bytes\n");
  u8g2log.print("PSRAM size: " + String(ESP.getPsramSize()) + " bytes\n");
  delay(300);
  u8g2log.print("CPU freq: " + String(ESP.getCpuFreqMHz()) + " MHz\n");
  u8g2log.print("Chip model: " + String(ESP.getChipModel()) + " rev " + String(ESP.getChipRevision()) + "\n");
  delay(300);

  // WiFi setup
  Serial.printf("Connecting to WiFi" );
  u8g2log.print("Connecting to WiFi...");
  u8g2log.setRedrawMode(1);

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  WiFi.setHostname("KitchenRadio2");
  WiFi.disconnect();

  WiFi.begin(CONF_WIFI_SSID, CONF_WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED)
  {
      u8g2log.print(".");
      Serial.print('.');
      delay(500);
  }

  char ipaddr[32];
  IPAddress localIp = WiFi.localIP();
  snprintf(ipaddr, sizeof(ipaddr), "%d.%d.%d.%d", localIp[0], localIp[1], localIp[2], localIp[3]);
  information.system.IPAddress = ipaddr;

  u8g2log.setRedrawMode(0);
  u8g2log.print("\nConnected! (" + (information.system.IPAddress) + ")\n");
  u8g2log.print("RSSI: " + String(WiFi.RSSI()) + " dBm\n");

  delay(300);

  //u8g2log.setRedrawMode(0);

  Serial.printf("\nTotal heap: %d", ESP.getHeapSize());
  Serial.printf("\nFree heap: %d", ESP.getFreeHeap());
  Serial.printf("\nTotal PSRAM: %d", ESP.getPsramSize());
  Serial.printf("\nFree PSRAM: %d", ESP.getFreePsram());
  Serial.printf("\nCPU freq: %d", ESP.getCpuFreqMHz());

  Serial.println(WiFi.localIP());

  // PSRAM buffer
  circBuffer.resize(CIRCBUFFER_SIZE);

  // Codec
  Serial.print("Starting vs1053\n");
  u8g2log.print("Starting codec...\n");
  audioplayer_init();

  if(player.isChipConnected())
  {
    u8g2log.print("VS1053 found\n");
  }
  else
  {
    u8g2log.print("Error: VS1053 not found!\n");
  }

  delay(500);

  // Tickers
  ticker_100ms_ref.attach(0.1, ticker_100ms);
  ticker_1000ms_ref.attach(1.0, ticker_1000ms);


  // Normal log
  u8g2log2.begin(U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
}

//int cnt = 0;
//uint8_t i=0;//
uint8_t hour;
uint8_t secs;

#define POS_CLOCK 210

uint32_t prev_millis = 0;

void loop() 
{


// TODO change to flag
  if((millis() - prev_millis > 1000) || (updateLog))
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
      
      if(updateLog)
      {
        u8g2log2.print(logline);
        
        updateLog = false;
      }
      u8g2.drawLog(100,2,u8g2log2);

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
    
    printLogLine("Volume: " + String(front_pot_vol));
  }

  if(flags.frontPanel.buttonOffPressed)
  {
      flags.frontPanel.buttonOffPressed = false;
      //set_sound_mode(SOUNDMODE_OFF);
      Serial.println("OFF");
      printLogLine("Sound off");
      front_led_off(LED_WEBRADIO);
      front_led_off(LED_BLUETOOTH);
  }
  if(flags.frontPanel.buttonRadioPressed)
  {
      flags.frontPanel.buttonRadioPressed = false;
      Serial.println("radio");
      printLogLine("Radio");
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
    printLogLine("Bluetooth");
    Serial.println("bluetooth");
  }

  if(flags.frontPanel.buttonSystemPressed)
  {
    flags.frontPanel.buttonSystemPressed = false;
    printLogLine("System");
  }
  
  if(flags.frontPanel.buttonAlarmPressed)
  {
    flags.frontPanel.buttonAlarmPressed = false;
    printLogLine("Alarm");
  }
  
  if(flags.frontPanel.buttonLampPressed)
  {
    flags.frontPanel.buttonLampPressed = false;
    printLogLine("Lamp");
  }

//  player.playChunk(sampleMp3, sizeof(sampleMp3));
 
}

