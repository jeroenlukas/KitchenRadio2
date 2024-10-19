#include <Arduino.h>

#include <Ticker.h>

#include <U8g2lib.h>

#include <WiFi.h>
#include <VS1053.h>
#include <SPI.h>

#include "log.h"
#include "configuration/config.h"
#include "information/krInfo.h"
#include "hmi/krFrontpanel.h"
#include "flags.h"



//Via tutorial from 
SPIClass *hspi = NULL;

WiFiClient client;

Ticker ticker_100ms_ref;

U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ HSPI_CS, /* dc=*/ HSPI_DC, /* reset=*/ 9);	// Enable U8G2_16BIT in u8g2.h


uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);


uint8_t mp3buff[64];
const char *host = "icecast.omroep.nl";
    const char *path = "/radio1-bb-mp3";
    int httpPort = 80;/// 8563;

void ticker_100ms()
{
    //webserver_handleclient();
    // Read pots
    front_read_pots();

  
}

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial.print("hoiix");


  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
  //pinMode(hspi->pinSS(), OUTPUT);

  frontpanel_setup();

  u8g2.begin(); 
  u8g2.setFont(U8LOG_FONT);
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8g2log.setRedrawMode(0);

  u8g2log.print("=== KitchenRadio2! ===\n");
 


  // delay(100); 
  /*u8g2.firstPage();
  do
  {
  u8g2.drawHLine(0,39,256);
  u8g2.setFont(u8g2_font_smallsimple_te);
  u8g2.setCursor(4,14);
  u8g2.drawStr(10,10, "Connecting...");
  } while ( u8g2.nextPage() );
*/
//  delay(500);
  // WiFi
    
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
    u8g2log.print("\nConnected! (");
    u8g2log.print(information.system.IPAddress);
    u8g2log.print(")\n");





   // u8g2.drawStr(20,20, "OK!");
   // delay(400);
    Serial.println(WiFi.localIP());

    Serial.print("Starting vs1053\n");
    u8g2log.print("Starting codec...\n");
    player.begin();
    if(player.isChipConnected())
    {
      u8g2log.print("VS1053 found\n");
    }
    else
    {
      u8g2log.print("ERROR: VS1053 not found\n");
    }
    player.loadDefaultVs1053Patches();
    player.switchToMp3Mode(); // optional, some boards require this
    player.setVolume(80);

    delay(2000);


    

       Serial.print("connecting to ");
    Serial.println(host);

    if (!client.connect(host, httpPort)) {
        Serial.println("Connection failed");
        return;
    }

    Serial.print("Requesting stream: ");
    Serial.println(path);

    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

  ticker_100ms_ref.attach(0.1, ticker_100ms);

}

//int cnt = 0;
//uint8_t i=0;//
uint8_t hour;
uint8_t secs;

#define POS_CLOCK 210

uint32_t prev_millis = 0;

void loop() 
{



if(millis() - prev_millis > 1000)
{
  prev_millis = millis();
  // put your main code here, to run repeatedly:
  u8g2.firstPage();
  do {
    
    // Clock
    u8g2.setFont(FONT_S);
    u8g2.drawStr(3, 24, (information.system.IPAddress).c_str());
    String s = "RSSI: " + String(WiFi.RSSI()) + " dBm";
    u8g2.drawStr(3, 34, s.c_str());

    u8g2.setFont(u8g2_font_lastapprenticebold_tr);
    u8g2.setCursor(POS_CLOCK + 4,14);
    u8g2.print(u8x8_u8toa(hour,2)); //u8x8_u8toa(cnt,3));
    u8g2.drawStr(POS_CLOCK + 22, 12, ":");
    u8g2.setCursor(POS_CLOCK + 29, 14);
    u8g2.print(u8x8_u8toa(secs,2));


    u8g2.setFont(u8g2_font_lastapprenticebold_tr);
    u8g2.drawStr(2, 62, "NPO Radio 7");

    // Log window
    u8g2.setFont(FONT_S);
    //u8g2log.
    
    u8g2.drawLog(100,2,u8g2log);
    

  } while ( u8g2.nextPage() );
  //Serial.println(secs);
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


if (!client.connected()) {
      Serial.println("Reconnecting...");
      if (client.connect(host, httpPort)) {
          client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Connection: close\r\n\r\n");
      }
  }

  if (client.available() > 0) {
      // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
      uint8_t bytesread = client.read(mp3buff, 64);
      player.playChunk(mp3buff, bytesread);
        
  }

// -------------------------=== FLAGS ===--------------------------

if (flags.frontPanel.volumePotChanged)
{
  flags.frontPanel.volumePotChanged = false;
  player.setVolume(log(front_pot_vol + 1) / log(127) * 100);
  Serial.println(front_pot_vol);
  u8g2log.setRedrawMode(0);
  u8g2log.print("volume changed:\n");
  u8g2log.print(front_pot_vol);
  u8g2log.print("\n");
}

if(flags.frontPanel.buttonOffPressed)
{
    flags.frontPanel.buttonOffPressed = false;
    //set_sound_mode(SOUNDMODE_OFF);
    Serial.println("OFF");
    front_led_off(LED_WEBRADIO);
    front_led_off(LED_BLUETOOTH);
}
if(flags.frontPanel.buttonRadioPressed)
{
    flags.frontPanel.buttonRadioPressed = false;
    Serial.println("radio");
    //set_sound_mode(SOUNDMODE_WEBRADIO);
    front_led_on(LED_WEBRADIO);
    front_led_off(LED_BLUETOOTH);
}

if(flags.frontPanel.buttonBluetoothPressed)
{
  flags.frontPanel.buttonBluetoothPressed = false;
  front_led_off(LED_WEBRADIO);
  front_led_on(LED_BLUETOOTH);
  Serial.println("bluetooth");
}

//  player.playChunk(sampleMp3, sizeof(sampleMp3));
 
}

