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

int16_t display_audio_title_scroll_offset = 0;
bool display_audio_title_scroll_dir = true;
uint16_t display_audio_title_width = 0;

String mitem_lamp_state_desc[2] = {"off", "on"};
String mitem_lamp_effecttype_desc[2] = {"off", "color fade"};

void display_draw_menu();
void display_draw_menu_footer(uint16_t mitem_min, uint16_t mitem_max);
void display_draw_menu_home();
void display_draw_menu_lamp();
void display_draw_menu_system();
void display_set_brightness(uint8_t brightness);
void display_set_brightness_auto();
void display_update_scroll_offset();


U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ HSPI_CS, /* dc=*/ HSPI_DC, /* reset=*/ 9);	// Enable U8G2_16BIT in u8g2.h

String convertTime(uint32_t timeInSeconds)
{  
  uint8_t seconds = timeInSeconds % 60;
  timeInSeconds /= 60;
  uint8_t minutes = timeInSeconds % 60;
  timeInSeconds /= 60;
  uint8_t hours = timeInSeconds % 24;
  uint8_t days = timeInSeconds / 24;
  String res;
  if(days > 0) res.concat(String(days) + "d ");
  if(hours > 0) res.concat(String(hours) + "h");
  if(minutes > 0) res.concat(String(minutes) + "m");
  res.concat(String(seconds) + "s");
  //String res = String(days) + "d" + String(hours) + "h" + String(minutes) + "m" + String(seconds) + "s";
  return res;
}

void display_update_scroll_offset()
{
  #define BOX_WIDTH (224 - POSX_AUDIO)
  if(display_audio_title_width < BOX_WIDTH)
  {
    display_audio_title_scroll_offset = 0;
    return;
  }

  if(display_audio_title_scroll_dir)
  {
    display_audio_title_scroll_offset+=2;
  }
  else
    display_audio_title_scroll_offset-=2;

  // Reverse 
  if(display_audio_title_scroll_offset > 2)
    display_audio_title_scroll_dir = !display_audio_title_scroll_dir;

  else if(display_audio_title_scroll_offset < ((BOX_WIDTH - display_audio_title_width)-3))
    display_audio_title_scroll_dir = !display_audio_title_scroll_dir;

  

}

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

            u8g2.drawGlyph(3, 35, weather_icon_to_glyph(information.weather.icon));
            u8g2.setFont(u8g2_font_lastapprenticebold_te);
            uint8_t w = u8g2.drawStr(42, 18,(String(information.weather.temperature,1) + "  C").c_str());
            u8g2.drawGlyph((42 + w) - 13, 18, 0x00b0);
            u8g2.setFont(FONT_M);
            u8g2.drawStr(42, 28,(String(information.weather.windSpeedBft) + " Bft").c_str());
            u8g2.drawStr(42, 38,(String(information.weather.stateShort)).c_str());
            u8g2.setFont(FONT_S);
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
          u8g2.drawLine(0, 44, 256, 44);

          // Sound mode / Station / Bluetooth audio title/artist
          switch(audioplayer_soundMode)
          {
            case SOUNDMODE_OFF:            
              break;
            case SOUNDMODE_WEBRADIO:
              u8g2.drawXBM(POSX_AUDIO_ICON, POSY_AUDIO_ICON-15, xbm_radio_width, xbm_radio_height, xbm_radio_bits);
              
              // Draw buffer fill percentage, station index + count
              u8g2.setFont(FONT_S);
              u8g2.drawStr(POSX_AUDIO -20, POSY_AUDIO-7, (String(information.webRadio.station_index + 1) + "/" + String(information.webRadio.station_count)).c_str() );
              u8g2.drawStr(POSX_AUDIO -20, POSY_AUDIO+1, (String(information.webRadio.buffer_pct) + "%").c_str() );

              // Draw station name in clipwindow
              u8g2.setFont(FONT_AUDIO);
              u8g2.setClipWindow(POSX_AUDIO, 43, 224, 64);
              display_audio_title_width = u8g2.drawStr(POSX_AUDIO + display_audio_title_scroll_offset, POSY_AUDIO, information.webRadio.station_name.c_str());
              u8g2.setMaxClipWindow();
              break;
            case SOUNDMODE_BLUETOOTH:
              u8g2.drawXBM(POSX_AUDIO_ICON, POSY_AUDIO_ICON-16, xbm_bluetooth_width, xbm_bluetooth_height, xbm_bluetooth_bits);

              // Draw bluetooth title in clipwindow
              u8g2.setFont(FONT_AUDIO);
              u8g2.setClipWindow(POSX_AUDIO, 43, 224, 64);              
              if(information.audioPlayer.bluetoothArtist != "")
                display_audio_title_width = u8g2.drawStr(POSX_AUDIO + display_audio_title_scroll_offset, POSY_AUDIO, String(information.audioPlayer.bluetoothArtist + " - " + information.audioPlayer.bluetoothTitle).c_str());
              else
                display_audio_title_width = u8g2.drawStr(POSX_AUDIO + display_audio_title_scroll_offset, POSY_AUDIO, String(information.audioPlayer.bluetoothConnectionStateStr).c_str());
              u8g2.setMaxClipWindow();
              
              // Draw audio state icon
              u8g2.setFont(u8g2_font_twelvedings_t_all);
              switch(information.audioPlayer.bluetoothMode)
              {
                case KCX_OFF:                  
                  u8g2.drawGlyph(POSX_AUDIO - 20, POSY_AUDIO, 0);
                  break;
                case KCX_NOTCONNECTED:
                  u8g2.drawGlyph(POSX_AUDIO - 20, POSY_AUDIO, 63);
                  break;
                case KCX_PAUSED:
                  u8g2.drawGlyph(POSX_AUDIO - 20, POSY_AUDIO, 69);
                  break;
                case KCX_PLAYING:
                  u8g2.drawGlyph(POSX_AUDIO - 20, POSY_AUDIO, 68);
                  break;
                case KCX_UNKNOWN:            
                  u8g2.drawStr(POSX_AUDIO - 20, POSY_AUDIO, "?");
                  break;
              }
              break;
          }
          //u8g2.setMaxClipWindow();

          // Log window
          log_debug_draw();

          // Volume          
          u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
          if(information.audioPlayer.mute)
            u8g2.drawGlyph(228, POSY_AUDIO, 279);
          else
            u8g2.drawGlyph(228, POSY_AUDIO, 277);
          u8g2.setFont(FONT_S);
          u8g2.drawStr(238, POSY_AUDIO-1, (String(information.audioPlayer.volume) + "%").c_str());
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
  
  u8g2.setFont(FONT_S);

  switch(menuitem)
  {
    case MITEM_SYSTEM_INFO:
      u8g2.drawStr(10, 12, String(information.device_name).c_str());
      u8g2.drawStr(10, 22, "IP: ");         u8g2.drawStr(70, 22, information.system.IPAddress.c_str());
      u8g2.drawStr(10, 32, "RSSI:");        u8g2.drawStr(70, 32, (String(information.system.wifiRSSI) + " dBm").c_str());      
      u8g2.drawStr(10, 42, "BT RSSI:");     u8g2.drawStr(70, 42, (String(information.audioPlayer.bluetoothRSSI) + " dBm").c_str());      
      
      u8g2.drawStr(150, 12, "Uptime:");     u8g2.drawStr(200, 12, convertTime(information.system.uptimeSeconds).c_str());
      u8g2.drawStr(150, 22, "Amb.light:");  u8g2.drawStr(200, 22, (String(information.system.ldr) + "%").c_str());
      
      break;
    case MITEM_SYSTEM_WEATHER:
      u8g2.drawStr(10, 12, "Wind:");       u8g2.drawStr(70, 12, (String(information.weather.windSpeedKmh) + "km/h").c_str());
      u8g2.drawStr(10, 22, "Temp:");       u8g2.drawStr(70, 22, (String(information.weather.temperature) + " 'C").c_str());
      u8g2.drawStr(10, 32, "Feels like:"); u8g2.drawStr(70, 32, (String(information.weather.temperature_feelslike) + " 'C").c_str());
      u8g2.drawStr(10, 42, "Humidity:");   u8g2.drawStr(70, 42, (String(information.weather.humidity) + "%").c_str());

      u8g2.drawStr(150, 12, "Pressure:");  u8g2.drawStr(200, 12, (String(information.weather.pressure) + " hPa").c_str());
      u8g2.drawStr(150, 22, "Sunrise:");   u8g2.drawStr(200, 22, (String(information.weather.sunrise_str)).c_str());
      u8g2.drawStr(150, 32, "Sunset:");    u8g2.drawStr(200, 32, (String(information.weather.sunset_str)).c_str());

      
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