#include <Arduino.h>
#include <U8g2lib.h>
#include <Ticker.h>

#include "configuration/config.h"
#include "configuration/constants.h"
#include "information/krInfo.h"
#include "information/krWeather.h"
#include "settings/krSettings.h"
#include "audioplayer/krAudioplayer.h"
#include "logger.h"
#include "hmi/krDisplay.h"

#include "hmi/xbmIcons.h"

#include "hmi/u8g2_font_climacons_40.h"

int menu = MENU_HOME;
int menuitem = 0;

String mitem_lamp_state_desc[2] = {"off", "on"};
String mitem_lamp_effecttype_desc[2] = {"off", "color fade"};

void display_draw_menu();
void display_draw_menu_footer(uint16_t mitem_min, uint16_t mitem_max);
void display_draw_menu_home();
void display_draw_menu_lamp();
void display_draw_menu_system();
void display_set_brightness(uint8_t brightness);
void display_set_brightness_auto();


U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ HSPI_CS, /* dc=*/ HSPI_DC, /* reset=*/ 9);	// Enable U8G2_16BIT in u8g2.h

void display_set_brightness(uint8_t brightness)
{
  // Contrast (0-100)
  uint8_t contrast = map(brightness, 0, 100, 0, 50);
  u8g2.setContrast(contrast);

  // Pre charge voltage (0-31) 0 is 0ff
  uint8_t pcv = map(brightness, 0, 100, 0, 31);

  u8g2.sendF("ca", 0xBB, pcv);  
}

void display_set_brightness_auto()
{
  //uint8_t brightness = map(information.system.ldr, 0, 100, CONF_DISPLAY_AUTO_BRIGHTNESS_MIN, CONF_DISPLAY_AUTO_BRIGHTNESS_MAX);
  
  int br_max = int(settings["display"]["brightness_max"]);
  int br_min = int(settings["display"]["brightness_min"]);

  if(br_max == 0) br_max = 100;

  

  uint8_t brightness = map(information.system.ldr, 0, 100, br_min, br_max);
  
  display_set_brightness(brightness);
  //log_debug("Setting birghtness to " + String(brightness));
}

void display_draw_menu()
{


  u8g2.firstPage();

    // This draws the main screen. Only screen related stuff should be done here.
    do {
      switch(menu)
      {
        case MENU_HOME:        
          display_draw_menu_home();          
          break; 

        case MENU_LAMP:        
          display_draw_menu_lamp();
          break;

        case MENU_SYSTEM:
          display_draw_menu_system();
          break;

        default:
          break;
      }
    } while ( u8g2.nextPage() );
}

void display_draw_menu_home()
{
    if(settings["display"]["homedisplay"] == "debug")
          {
            u8g2.setFont(FONT_S);
            u8g2.drawStr(3, 6, (information.weather.stateShort).c_str());
            u8g2.drawStr(3, 14, ("Temp: " + String(information.weather.temperature) + "\xb0" + "C").c_str());
            u8g2.drawStr(3, 22, ("Wind: " + String(information.weather.windSpeedKmh) + " kmh").c_str());
            u8g2.drawStr(3, 30, ("RSSI: " + String(information.system.wifiRSSI) + " dBm").c_str());
            u8g2.drawStr(3, 38, ("Buf: " + String(circBuffer.available()) + " B").c_str());
          }
          else if(settings["display"]["homedisplay"] == "normal")
          {
            u8g2.setFont(u8g2_font_climacons_40);
            int weatherglyph = 0;
            // https://openweathermap.org/weather-conditions

            //u8g2.drawGlyph(5, 35, weather_statecode_to_glyph(information.weather.stateCode));
            u8g2.drawGlyph(5, 35, weather_icon_to_glyph(information.weather.icon));
            u8g2.setFont(u8g2_font_lastapprenticebold_tr);
            u8g2.drawStr(42, 18,(String(information.weather.temperature,1) + " C").c_str());
            u8g2.setFont(FONT_M);
            u8g2.drawStr(42, 28,(String(information.weather.windSpeedBft) + " Bft").c_str());
            u8g2.drawStr(42, 38,(String(information.weather.stateShort)).c_str());
            u8g2.setFont(FONT_S);
            if(audioplayer_soundMode != SOUNDMODE_OFF)
              u8g2.drawStr(POSX_CLOCK + 20, 60, ("B: " + String(circBuffer.available()) + " B").c_str());

            u8g2.drawStr(155, 60, ("Vol: " + String(information.audioPlayer.volume) + "%").c_str());
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
            
              //u8g2.setFont(u8g2_font_lastapprenticebold_tr);
              //u8g2.drawStr(POSX_AUDIO, POSY_AUDIO, "");
              break;
            case SOUNDMODE_WEBRADIO:
              u8g2.drawXBM(POSX_AUDIO_ICON, POSY_AUDIO_ICON-16, xbm_radio_width, xbm_radio_height, xbm_radio_bits);
              u8g2.setFont(u8g2_font_lastapprenticebold_tr);
              u8g2.drawStr(POSX_AUDIO, POSY_AUDIO, (String(information.webRadio.station_index + 1) + "/" + String(information.webRadio.station_count) + " " + (information.webRadio.station_name)).c_str());
              break;
            case SOUNDMODE_BLUETOOTH:
              u8g2.drawXBM(POSX_AUDIO_ICON, POSY_AUDIO_ICON-16, xbm_bluetooth_width, xbm_bluetooth_height, xbm_bluetooth_bits);

              u8g2.setFont(u8g2_font_lastapprenticebold_tr);
              u8g2.drawStr(POSX_AUDIO, POSY_AUDIO, information.audioPlayer.bluetoothTitle.c_str());
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
}

void display_draw_menu_footer(uint16_t mitem_min, uint16_t mitem_max)
{
  u8g2.drawLine(0, 48, 256, 48);

  u8g2.setFont(u8g2_font_6x12_m_symbols);
  u8g2.drawGlyph(10, 62, 8626); // Back

  if(menuitem > mitem_min)
  u8g2.drawGlyph(82, 62, 9650); // Up

  if(menuitem < mitem_max)
    u8g2.drawGlyph(146, 62, 9660); // Down
}

void display_draw_menu_lamp()
{
  display_draw_menu_footer(MITEM_LAMP_MIN, MITEM_LAMP_MAX);

  u8g2.setFont(FONT_M);
  u8g2.drawStr(10, 10, "Lamp");
  u8g2.setFont(FONT_S);
    
  switch(menuitem)
  {
    case MITEM_LAMP_STATE:
      u8g2.drawStr(10, 30, "State:");
      u8g2.drawStr(80, 30, mitem_lamp_state_desc[information.lamp.state].c_str());
      break;
    case MITEM_LAMP_HUE:
      u8g2.drawStr(10, 30, "Hue:");
      u8g2.drawStr(80, 30, String(information.lamp.hue, 3).c_str());
      break;
    case MITEM_LAMP_SATURATION:
      u8g2.drawStr(10, 30, "Saturation:");
      u8g2.drawStr(80, 30, String(information.lamp.saturation, 3).c_str());
      break;
    case MITEM_LAMP_LIGHTNESS:
      u8g2.drawStr(10, 30, "Brightness:");
      u8g2.drawStr(80, 30, String(information.lamp.lightness, 3).c_str());
      break;
    case MITEM_LAMP_EFFECTTYPE:
      u8g2.drawStr(10, 30, "Effect type:");
      //u8g2.drawStr(80, 30, String(information.lamp.effect_type).c_str());
      u8g2.drawStr(80, 30, mitem_lamp_effecttype_desc[information.lamp.effect_type].c_str());
      break;
    case MITEM_LAMP_EFFECTSPEED:
      u8g2.drawStr(10, 30, "Effect speed:");
      u8g2.drawStr(80, 30, String((information.lamp.effect_speed), 4).c_str());
      break;
    default:
      break;
  }
}

void display_draw_menu_system()
{
  display_draw_menu_footer(MITEM_SYSTEM_MIN, MITEM_SYSTEM_MAX);

  u8g2.setFont(FONT_M);
  u8g2.drawStr(10, 10, "System");
  u8g2.setFont(FONT_S);

  switch(menuitem)
  {
    case MITEM_SYSTEM_INFO:
      u8g2.drawStr(10, 22, "IP: ");         u8g2.drawStr(70, 22, information.system.IPAddress.c_str());
      u8g2.drawStr(10, 32, "RSSI:");        u8g2.drawStr(70, 32, (String(information.system.wifiRSSI) + " dBm").c_str());
      u8g2.drawStr(10, 42, "Uptime:");      u8g2.drawStr(70, 42, (String(information.system.uptimeSeconds) + " sec").c_str());
      u8g2.drawStr(150, 22, "Amb.light:");  u8g2.drawStr(200, 22, (String(information.system.ldr) + "%").c_str());
      u8g2.drawStr(150, 32, "Wind:");       u8g2.drawStr(200, 32, (String(information.weather.windSpeedKmh) + "kmh").c_str());
      break;
    case MITEM_SYSTEM_BASS:
      u8g2.drawStr(10, 30, "Bass:");        u8g2.drawStr(80, 30, String(information.audioPlayer.bass).c_str());
      break;
    case MITEM_SYSTEM_TREBLE:
      u8g2.drawStr(10, 30, "Treble:");      u8g2.drawStr(80, 30, String(information.audioPlayer.treble).c_str());
      break;
    default:
      break;
  }

  
}