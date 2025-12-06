#ifndef KR_CLI_H
#define KR_CLI_H

#include <Arduino.h>

extern void cli_init();
extern void cli_parse(String input);
extern void cli_handle();

#endif