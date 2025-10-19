#ifndef FLAGS_H
#define FLAGS_H


#include "hmi/flagsFrontPanel.h"
#include "flagsMain.h"

/*
This file contains event flags, which can be set and read throughout the system to track events.
Components that fire events should contain a flags***.h file, which contains a struct containing the flags.
*/

class Flags
{
    public:

    FlagsMain main;

    FlagsFrontPanel frontPanel;
    


};

extern Flags flags;

extern int menu;
extern int menuitem;

#endif