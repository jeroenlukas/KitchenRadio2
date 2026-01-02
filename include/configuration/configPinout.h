#ifndef CONFIGPINOUT_H
#define CONFIGPINOUT_H

#define KR_REV2

#ifdef KR_REV2

#define HSPI_SCK        12 
#define HSPI_MISO       13
#define HSPI_MOSI       11
#define HSPI_CS         48
#define HSPI_DC         10

// VS1053 board (SPI connected in a standard way)
#define VS1053_CS       14
#define VS1053_DCS      47
#define VS1053_DREQ     21

// UART for comms with the KCX bluetooth module
// Note, this is seen from the ESP's perspective, so TX = ESP output and RX = ESP input
#define KCX_TX          43
#define KCX_RX          44 
#define KCX_CONNECT     42


// MCP I/O
// https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library/tree/master?tab=readme-ov-file#pin-addressing
#define MCP_INTA            15
#define MCP_INTB            16

// LEDs
#define MCP_LED_WEBRADIO   3 //6
#define MCP_LED_BLUETOOTH  2 //7
#define MCP_LED_ALARM      1 //15
#define MCP_LED_LAMP       0 //16

// Buttons
#define MCP_BTN_OFF        8
#define MCP_BTN_WEBRADIO   9
#define MCP_BTN_BLUETOOTH   10
#define MCP_BTN_ALARM       11
#define MCP_BTN_LAMP        12
#define MCP_BTN_SYSTEM      13
#define MCP_BTN_ENC1        4
#define MCP_BTN_ENC2        5

// Buttons
//#define BUTTONS         //4

// Pot meters
//#define POT_VOLUME      //5

// Encoder
#define ROTARY1_A        7 //38
#define ROTARY1_B        6 //39
#define BUTTON_ENCODER  //40
#define ROTARY2_A        5
#define ROTARY2_B        4

// I2C
#define PIN_SCL         1
#define PIN_SDA         2

// LDR
#define LDR             17

// LED ring
#define PIN_LED_RING    38

// Power amp
#define PIN_PA_MUTE     39

// Buzzer
#define PIN_BUZZER      18

// I2S
#define PIN_SLAVEI2S_SCK     42
#define PIN_SLAVEI2S_SD      41
#define PIN_SLAVEI2S_WS      40

#define PIN_UART_BT_TX  43
#define PIN_UART_BT_RX  44

#endif

#ifdef KR_REV0
// SPI bus
#define HSPI_SCK        12 
#define HSPI_MISO       13
#define HSPI_MOSI       11
#define HSPI_CS         48
#define HSPI_DC         10

// VS1053 board (SPI connected in a standard way)
#define VS1053_CS       14
#define VS1053_DCS      47
#define VS1053_DREQ     21

// UART for comms with the KCX bluetooth module
// Note, this is seen from the ESP's perspective, so TX = ESP output and RX = ESP input
#define KCX_TX          43
#define KCX_RX          44
#define KCX_CONNECT     42


// LEDs
#define LED_WEBRADIO    6
#define LED_BLUETOOTH   7
#define LED_ALARM       15
#define LED_LAMP        16

// Buttons
#define BUTTONS         4

// Pot meters
#define POT_VOLUME      5

// Encoder
#define ROTARY_A        38
#define ROTARY_B        39
#define BUTTON_ENCODER  40

// LDR
#define LDR             17

// LED ring
#define PIN_LED_RING    18

#endif

#endif