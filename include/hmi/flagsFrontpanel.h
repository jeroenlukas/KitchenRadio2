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

    bool buttonAnyPressed;

};

//void frontpanel_setup();
//void front_multibuttons_loop();

#endif