#include <Arduino.h>
#include <U8g2lib.h>

#include <WiFi.h>
#include <VS1053.h>
#include <SPI.h>

#include "SampleMp3.h"

#define HSPI_SCK 12 
#define HSPI_MISO 13
#define HSPI_MOSI 11
#define HSPI_CS 48
#define HSPI_DC 10
#define VS1053_CS 14
#define VS1053_DCS 47
#define VS1053_DREQ 21

//Via tutorial from 
SPIClass *hspi = NULL;

WiFiClient client;

U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ HSPI_CS, /* dc=*/ HSPI_DC, /* reset=*/ 9);	// Enable U8G2_16BIT in u8g2.h

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);


uint8_t mp3buff[64];
const char *host = "icecast.omroep.nl";
    const char *path = "/radio1-bb-mp3";
    int httpPort = 80;/// 8563;

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial.print("hoiix");


  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
  //pinMode(hspi->pinSS(), OUTPUT);

  u8g2.begin(); 
   delay(100); 
  u8g2.firstPage();
  do
  {
  u8g2.drawHLine(0,39,256);
  u8g2.setFont(u8g2_font_smallsimple_te);
  u8g2.setCursor(4,14);
  u8g2.drawStr(10,10, "Connecting...");
  } while ( u8g2.nextPage() );

  delay(500);
  // WiFi
    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
    WiFi.setHostname("KitchenRadio2");
    WiFi.disconnect();

    WiFi.begin("Rinus", "scheldestraat");
    Serial.printf("Connecting to WiFi" );
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(500);
    }

    u8g2.drawStr(20,20, "OK!");
    delay(400);
    Serial.println(WiFi.localIP());

    Serial.print("Starting vs1053");
    player.begin();
    player.loadDefaultVs1053Patches();
    player.switchToMp3Mode(); // optional, some boards require this
    player.setVolume(100);


    

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
}

//int cnt = 0;
//uint8_t i=0;//
uint8_t hour;
uint8_t secs;

#define POS_CLOCK 210

uint32_t prev_millis = 0;

void loop() {


  if(millis() - prev_millis > 1000)
  {
    prev_millis = millis();
    // put your main code here, to run repeatedly:
    u8g2.firstPage();
    do {
      u8g2.drawHLine(0,39,256);
      //u8g2.drawHLine(0,31,10);    
      u8g2.setContrast(1);

      u8g2.setFont(u8g2_font_lastapprenticebold_tr);

      u8g2.setCursor(POS_CLOCK + 4,14);
      u8g2.print(u8x8_u8toa(hour,2)); //u8x8_u8toa(cnt,3));

      u8g2.drawStr(POS_CLOCK + 22, 12, ":");

      u8g2.setCursor(POS_CLOCK + 29, 14);
      u8g2.print(u8x8_u8toa(secs,2));

      u8g2.setFont(u8g2_font_smallsimple_te);
      u8g2.drawStr(POS_CLOCK, 24, "  Zaterdag");
      u8g2.drawStr(POS_CLOCK, 34, " 5 Oktober");

      char ipaddr[32];
      IPAddress localIp = WiFi.localIP();
      snprintf(ipaddr, sizeof(ipaddr), "%d.%d.%d.%d", localIp[0], localIp[1], localIp[2], localIp[3]);

      u8g2.drawStr(3, 24, ipaddr);
      String s = "RSSI: " + String(WiFi.RSSI()) + " dBm";
      u8g2.drawStr(3, 34, s.c_str());
      //u8g2.print("RSSI: %d dBm", WiFi.RSSI());



      u8g2.setFont(u8g2_font_lastapprenticebold_tr);

      //u8g2.setCursor(2,16);
      u8g2.drawStr(2,14,"7.4C");
      //u8g2.drawStr(20,14, 176);

      //u8g2.setFont(u8g2_font_t0_22b_te  );
      u8g2.drawStr(2, 62, "NPO Radio 5");


      u8g2.setFont(u8g2_font_streamline_all_t);
      u8g2.drawGlyph(234, 64, 432+5);
      

    } while ( u8g2.nextPage() );
    Serial.println(secs);
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
  //delay(2000);
 
  
 
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

//  player.playChunk(sampleMp3, sizeof(sampleMp3));
 
}

