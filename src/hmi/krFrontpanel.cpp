#include <Arduino.h>
//#include <RotaryEncoder.h>

#include <BfButtonManager.h>
#include <BfButton.h>
#include <RotaryEncoder.h>
#include <Adafruit_MCP23X17.h>
#include "configuration/config.h"
#include "hmi/krFrontPanel.h"
#include "information/krInfo.h"
#include "flags.h"

// To calibrate button ADC values
//#define CAL_BUTTONS

int adc_pot_vol = 0;

int prev_adc_pot_vol = 0;

int front_pot_vol = 0;

bool prev_button_encoder = true;

void front_handle();
void front_i2c_ping();
void front_ldr_read();

Adafruit_MCP23X17 mcp;

RotaryEncoder encoder1(ROTARY1_A, ROTARY1_B, RotaryEncoder::LatchMode::FOUR3);
RotaryEncoder encoder2(ROTARY2_A, ROTARY2_B, RotaryEncoder::LatchMode::FOUR3);






void front_init()
{
    // LDR
    pinMode(LDR, INPUT);

    // MCP interrupts
    pinMode(MCP_INTA, INPUT);
    pinMode(MCP_INTB, INPUT);

    Wire.setPins(PIN_SDA, PIN_SCL);
    Wire.begin();

    #ifdef MCP_PING
    delay(1000);
    front_i2c_ping();
    #endif
    
    mcp.begin_I2C();    

    //mcp.setupInterrupts(false, true, HIGH);
    

    // Buttons
    mcp.pinMode(MCP_BTN_OFF, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN_WEBRADIO, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN_BLUETOOTH, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN_SYSTEM, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN_ALARM, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN_LAMP, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN_ENC1, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN_ENC2, INPUT_PULLUP);

    // Interrupts are disabled by default

    mcp.setupInterruptPin(MCP_BTN_OFF, CHANGE);
    mcp.setupInterruptPin(MCP_BTN_WEBRADIO, CHANGE);
    //mcp.setupInterruptPin(MCP_BTN_BLUETOOTH, CHANGE);
    //mcp.setupInterruptPin(MCP_BTN_SYSTEM, CHANGE);
    //mcp.setupInterruptPin(MCP_BTN_ALARM, CHANGE);
    //mcp.setupInterruptPin(MCP_BTN_LAMP, CHANGE);
    mcp.setupInterruptPin(MCP_BTN_ENC1, CHANGE);
    mcp.setupInterruptPin(MCP_BTN_ENC2, CHANGE);

    // LEDs
    mcp.pinMode(MCP_LED_WEBRADIO, OUTPUT);
    mcp.pinMode(MCP_LED_BLUETOOTH, OUTPUT);    
    mcp.pinMode(MCP_LED_ALARM, OUTPUT);
    mcp.pinMode(MCP_LED_LAMP, OUTPUT);
    mcp.digitalWrite(MCP_LED_WEBRADIO, HIGH);    
    mcp.digitalWrite(MCP_LED_BLUETOOTH, HIGH);
    
    mcp.clearInterrupts();
}

void front_led_on(uint8_t led)
{
    digitalWrite(led, 1);
}

void front_led_off(uint8_t led)
{
    digitalWrite(led, 0);
}


void front_ldr_read()
{
    uint16_t adc = 4095 - analogRead(LDR);
    information.system.ldr = map(adc, 0, 4095, 0, 100);
}

void front_encoders_read()
{
    static int pos1 = 0;
    encoder1.tick();

    int newPos1 = encoder1.getPosition();
    if (pos1 != newPos1)
    {
        if ((int)(encoder1.getDirection()) == -1)
        {
            flags.frontPanel.encoder1TurnRight = true;
            flags.frontPanel.buttonAnyPressed = true;
        }
        else
        {
            flags.frontPanel.encoder1TurnLeft = true;            
            flags.frontPanel.buttonAnyPressed = true;
        }
        pos1 = newPos1;
    }

    static int pos2 = 0;
    encoder2.tick();

    int newPos2 = encoder2.getPosition();
    if (pos2 != newPos2)
    {
        if ((int)(encoder2.getDirection()) == 1)
        {
            flags.frontPanel.encoder2TurnRight = true;
            flags.frontPanel.buttonAnyPressed = true;
        }
        else
        {
            flags.frontPanel.encoder2TurnLeft = true;           
            flags.frontPanel.buttonAnyPressed = true;
        }
        pos2 = newPos2;
    }
}

void front_buttons_read()
{
    static bool mcp_inta_prev;

    int mcp_inta = !digitalRead(MCP_INTA); // Interrupt for encoder switches
    int mcp_intb = !digitalRead(MCP_INTB); // Interrupt
    
    if(mcp_inta || mcp_intb)
    {
        Serial.println();
       Serial.print("Interrupt detected on pin: ");
        Serial.println(mcp.getLastInterruptPin());
        
        //Serial.print("Pin states at time of interrupt: 0b");
        //Serial.println(mcp.getCapturedInterrupt(), 2);
        //delay(10);  // debounce
        // NOTE: If using DEFVAL, INT clears only if interrupt
        // condition does not exist.
        // See Fig 1-7 in datasheet.
        mcp.clearInterrupts();  // clear
    }


}

// Should be called from main loop
void front_handle(void)
{
    front_encoders_read();

    //front_buttons_read();

    //front_ldr_read();
}

// Helper function to ping I2C devices
void front_i2c_ping()
{
    Serial.println("Wire ping");
    delay(1000);

    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for(address = 1; address < 127; address++ ) 
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
        Serial.print("I2C device found at address 0x");
        if (address<16) {
            Serial.print("0");
        }
        Serial.println(address,HEX);
        nDevices++;
        }
        else if (error==4) {
        Serial.print("Unknow error at address 0x");
        if (address<16) {
            Serial.print("0");
        }
        Serial.println(address,HEX);
        }    
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found\n");
    }
    else {
        Serial.println("done\n");
    }
}