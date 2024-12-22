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

void cb_bootlog(cmd* c)
{
    Serial.print(bootlog);
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






}

void cli_parse(String input)
{
    kr_cli.parse(input);
}