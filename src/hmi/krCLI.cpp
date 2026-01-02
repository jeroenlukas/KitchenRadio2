#include <Arduino.h>
#include <LittleFS.h>

#include "hmi/krCLI.h"
#include "configuration/config.h"
#include "configuration/constants.h"

#include "audioplayer/krAudioplayer.h"
#include "audioplayer/kcx.h"
#include "information/krWeather.h"
#include "information/krTime.h"
#include "information/krInfo.h"
#include "logger.h"
#include "hmi/krLamp.h"
#include "hmi/krDisplay.h"
#include "hmi/krBuzzer.h"
#include "audioplayer/krI2S.h"

#include <SimpleCLI.h>

SimpleCLI kr_cli;

Command cmd_reset;
Command cmd_soundmode;
Command cmd_help;
Command cmd_volume;
Command cmd_lamp;
Command cmd_bootlog;
Command cmd_log;
Command cmd_effect;
Command cmd_cat;
Command cmd_fake;
Command cmd_oled;
Command cmd_buzzer;
Command cmd_bt;

void cli_handle();

void cb_reset(cmd* c) 
{
    Command cmd(c);

    Serial.println("I will be resetting!");

    delay(1000);

    ESP.restart();
}

void cb_soundmode(cmd* c)
{
    Command cmd(c);

    if(cmd.getArg("r").isSet()) audioplayer_set_soundmode(SOUNDMODE_WEBRADIO);
    else if(cmd.getArg("o").isSet()) audioplayer_set_soundmode(SOUNDMODE_OFF);
    else if(cmd.getArg("b").isSet()) audioplayer_set_soundmode(SOUNDMODE_BLUETOOTH);
    
    else Serial.println("Error: invalid soundmode");   
}

void cb_volume(cmd* c)
{
    Command cmd(c);    

    if(cmd.getArgument(0).isSet())
    {
        uint8_t volume = cmd.getArgument(0).getValue().toInt();
        audioplayer_setvolume(volume);
    }
}

void cb_lamp(cmd* c)
{
    Command cmd(c);
    if(cmd.getArgument("h").isSet())
    {
        Serial.println("Setting hue");
        float h = cmd.getArgument("h").getValue().toFloat();
        lamp_sethue(h);
    }
    if(cmd.getArgument("s").isSet())
    {
        Serial.println("setting saturation");
        float s = cmd.getArgument("s").getValue().toFloat();
        lamp_setsaturation(s);
    }
    if(cmd.getArgument("l").isSet())
    {
        Serial.println("setting lightness");
        float l = cmd.getArgument("l").getValue().toFloat();
        lamp_setlightness(l);
    }
    

}

void cb_effect(cmd* c)
{
    Command cmd(c);

    if(cmd.getArgument("t").isSet())
    {
        lamp_seteffecttype(cmd.getArgument("t").getValue().toInt());
    }
    if(cmd.getArgument("s").isSet())
    {
        lamp_seteffectspeed(cmd.getArgument("s").getValue().toFloat());
    }
}

void cb_fake(cmd* c)
{
    Command cmd(c);
    if(cmd.getArgument("weatherstate").isSet())
    {
        int code= cmd.getArgument("weatherstate").getValue().toInt();
        log_debug("Faking weatherstate to " + String(code) );
        information.weather.stateCode = code;
    }
}

void cb_oled(cmd* c) 
{
    Command cmd(c);

    if(cmd.getArgument("brightness").isSet())
    {
        int val = cmd.getArgument("brightness").getValue().toInt();
        log_debug("[OLED] Set brightness to " + String(val) + "%");
        display_set_brightness(val);
    }

    else if(cmd.getArgument("contrast").isSet())
    {
        int val = cmd.getArgument("contrast").getValue().toInt();
        log_debug("[OLED] Set contast to " + String(val));
        u8g2.setContrast(val);
    }
    
    else if(cmd.getArgument("on").isSet())
    {
        log_debug("[OLED] Turn whole OLED on");
        u8g2.sendF("c", 0xAF);
    }    
    else if(cmd.getArgument("off").isSet())
    {
        log_debug("[OLED] Turn whole OLED off");
        u8g2.sendF("c", 0xAE);
    }

    else if(cmd.getArgument("allon").isSet())
    {
        log_debug("[OLED] Turn all pixels on");
        u8g2.sendF("c", 0xA5);
    }
    else if(cmd.getArgument("alloff").isSet())
    {
        log_debug("[OLED] Turn whole OLED off");
        u8g2.sendF("c", 0xA6);
    }
    else if(cmd.getArgument("normal").isSet())
    {
        log_debug("[OLED] Turn OLED normal");
        u8g2.sendF("c", 0xA6);
    }
    else if(cmd.getArgument("inverse").isSet())
    {
        log_debug("[OLED] Turn OLED inverse");
        u8g2.sendF("c", 0xA7);
    }
    
    if(cmd.getArgument("mcurr").isSet())
    {
        int val = constrain(cmd.getArgument("mcurr").getValue().toInt(), 0b0000, 0b1111);
        log_debug("[OLED] Set master current byte to " + String(val));
        u8g2.sendF("ca", 0xC7, val);
    }
    if(cmd.getArgument("pcv").isSet())
    {
        int val = constrain(cmd.getArgument("pcv").getValue().toInt(), 0b00000, 0b11111);
        log_debug("[OLED] Set pre charge voltage byte to " + String(val));
        u8g2.sendF("ca", 0xBB, val);
    }
    if(cmd.getArgument("clock").isSet())
    {
        int val = constrain(cmd.getArgument("clock").getValue().toInt(), 0b00000000, 0b11111111);
        log_debug("[OLED] Set clock/osc byte to " + String(val));
        u8g2.sendF("ca", 0xB3, val);
    }

}

void cb_buzzer(cmd* c)
{
    log_debug("Beep");
    buzzer_beep(500);
    
    return;
}

void cb_bt(cmd* c)
{
    Command cmd(c);

    if(cmd.getArgument("reset").isSet())
    {
        slavei2s_send("AT+RESET");
    }
    if(cmd.getArgument("pause").isSet())
    {
        slavei2s_send("AT+PAUSE");
    }
    if(cmd.getArgument("play").isSet())
    {
        slavei2s_send("AT+PLAY");
    }
    if(cmd.getArgument("stop").isSet())
    {
        slavei2s_send("AT+STOP");
    }
    if(cmd.getArgument("start").isSet())
    {
        slavei2s_send("AT+START");
    }
    if(cmd.getArgument("end").isSet())
    {
        slavei2s_send("AT+END");
    }
    
    
    
}

void cb_bootlog(cmd* c)
{
    Serial.print(bootlog);

    return;
}

void cb_log(cmd* c)
{
    Command cmd(c);
    String line = cmd.getArgument(0).getValue();
    log_debug(line);
}

void cb_help(cmd* c)
{
    Serial.println("--- Commands ---");
    Serial.println(kr_cli.toString());
}

void cb_cat(cmd* c)
{
    Command cmd(c);
    String filename = cmd.getArgument(0).getValue();
    File file_cat = LittleFS.open(filename, "r");

    if(!file_cat)
    {
        Serial.print("Error: could not open file " + filename);
        return;
    }

    String file_content;
    Serial.println("\n--- " + filename + " ---\n");

    while(file_cat.available())
    {
        String data = file_cat.readString();
        //file_content += data;
        Serial.print(data);
    }
    Serial.print("\n--- end ---\n");

    file_cat.close();
}



void cb_error(cmd_error* e) {
    CommandError cmdError(e); // Create wrapper object

    Serial.print("ERROR: ");
    Serial.println(cmdError.toString());

    if (cmdError.hasCommand()) {
        Serial.print("Did you mean \"");
        Serial.print(cmdError.getCommand().toString());
        Serial.println("\"?");
    }
}

void cli_init(void)
{
    kr_cli.setOnError(cb_error);

    // > reset
    cmd_reset = kr_cli.addCmd("reset", cb_reset);
    cmd_reset.setDescription("- Reset the ESP32");

    // > soundmode
    cmd_soundmode = kr_cli.addCommand("soundmode", cb_soundmode);
    cmd_soundmode.addFlagArgument("o/ff");
    cmd_soundmode.addFlagArgument("r/adio");
    cmd_soundmode.addFlagArgument("b/luetooth");    
    cmd_soundmode.setDescription("- Set the soundmode");

    // > bootlog
    cmd_bootlog = kr_cli.addCommand("bootlog", cb_bootlog);
    cmd_bootlog.setDescription("- Print the boot log with timestamps");

    // > help
    cmd_help = kr_cli.addCmd("help", cb_help);
    cmd_help.setDescription("- Show this help");

    // > lamp
    cmd_lamp = kr_cli.addCmd("lamp", cb_lamp);
    cmd_lamp.addArgument("h/ue", "0.0");
    cmd_lamp.addArgument("s/aturation", "0.0");
    cmd_lamp.addArgument("l/ightness", "0.0");    
    cmd_lamp.setDescription("- Set the lamp hue (0.0 - 1.0), saturation (0.0 - 1.0) and/or lightness (0.0 - 0.5)");

    // > effect
    cmd_effect = kr_cli.addCmd("effect", cb_effect);
    cmd_effect.addArgument("t/ype", "0");
    cmd_effect.addArgument("s/peed", "0");
    cmd_effect.setDescription("- Set lamp effect and parameters");
    
    // > log
    cmd_log = kr_cli.addSingleArgCmd("log", cb_log);
    cmd_log.setDescription("- Print a debug message");

    // > cat
    cmd_cat = kr_cli.addSingleArgumentCommand("cat", cb_cat);
    cmd_cat.setDescription("- Print contents of a file. Example: cat /settings/stations.json");

    // > fake
    cmd_fake = kr_cli.addCmd("fake", cb_fake);
    cmd_fake.addArgument("weatherstate", "804");

    // > oled
    cmd_oled = kr_cli.addCmd("oled", cb_oled);
    cmd_oled.addArgument("b/rightness", "100");
    cmd_oled.addArgument("c/ontrast", "100");
    cmd_oled.addFlagArgument("on");
    cmd_oled.addFlagArgument("off");
    cmd_oled.addFlagArgument("allon");
    cmd_oled.addFlagArgument("alloff");
    cmd_oled.addFlagArgument("normal");
    cmd_oled.addFlagArgument("inverse");
    cmd_oled.addArgument("clock", "0");
    cmd_oled.addArgument("pcv", "0");    // Pre charge voltage
    cmd_oled.addArgument("mcurr", "0");  // Master current

    // > volume
    cmd_volume = kr_cli.addSingleArgCmd("volume", cb_volume);

    // > buzzer
    cmd_buzzer = kr_cli.addSingleArgCmd("buzzer", cb_buzzer);

    // > bt
    cmd_bt = kr_cli.addCmd("bt", cb_bt);
    cmd_bt.addFlagArgument("reset");
    cmd_bt.addFlagArgument("pause");
    cmd_bt.addFlagArgument("play");
    cmd_bt.addFlagArgument("stop");
    cmd_bt.addFlagArgument("start");
    cmd_bt.addFlagArgument("end");


}

void cli_parse(String input)
{
    kr_cli.parse(input);
}

void cli_handle()
{
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
    }
}