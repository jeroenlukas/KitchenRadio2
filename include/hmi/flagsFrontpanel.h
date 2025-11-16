#ifndef FLAGSFRONTPANEL_H
#define FLAGSFRONTPANEL_H

struct FlagsFrontPanel
{
    bool volumePotChanged;

    bool encoderButtonPressed;
    bool encoderTurnRight;
    bool encoderTurnLeft;

    bool buttonOffPressed;
    bool buttonRadioPressed;
    bool buttonBluetoothPressed;
    bool buttonSystemPressed;
    bool buttonAlarmPressed;
    bool buttonLampPressed;

    bool buttonOffLongPressed;
    bool buttonRadioLongPressed;
    bool buttonBluetoothLongPressed;
    bool buttonSystemLongPressed;
    bool buttonAlarmLongPressed;
    bool buttonLampLongPressed;

    bool buttonAnyPressed;

};

//void frontpanel_setup();
//void front_multibuttons_loop();

#endif