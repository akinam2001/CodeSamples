/* This function in embedded C was created to implement a state machine of an oven on a PIC32 microcontroller.
It updates the lights and LED board on the PIC32 to correspond to the state. It also reads in data from buttons 
and knobs on the controller. */

// Standard libraries
#include <stdio.h>

//CSE13E Support Library
#include "BOARD.h"
#include "Leds.h"
#include "Oled.h"
#include "Buttons.h"
#include "Adc.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

// **** Set any local typedefs here ****
//holds state of oven
typedef enum {
    SETUP, SELECTOR_CHANGE_PENDING, COOKING, RESET_PENDING, ALERT
} OvenState;

//holds what cooking mode the oven is in
typedef enum {
    BAKE = 0,                       //if bake, OvenMode is 0
    TOAST = 1,                      //if toast, OvenMode is 1
    BROIL = 2,                      //if broil, OvenMode is 2
} OvenMode;

//holds whether time or temp is selected
typedef enum {
    TIME = 0,                       //if time is selected, SelectorMode is 0
    TEMP = 1,                       //if temp is selected, SelectorMode is 0
} SelectorMode;

//holds all data for the oven
typedef struct {
    OvenState state;                //holds state of oven
    //add more members to this struct
    OvenMode mode;                  //holds mode of oven 
    int16_t cookTime;               //holds selected cook time 
    int16_t cookTimeRemaining;      //holds remaining cook time (incremented down)
    int16_t temperature;            //holds selected temperature 
    int16_t adcReading;             //holds adc reading
    SelectorMode selector;          //holds what is selected (time or temp)
    uint8_t buttonState;            //holds state of buttons
    uint8_t adcEvent;               //holds whether there is a change in ADC
    uint8_t buttonEvent;            //holds whether there is a change in button state

} OvenData;

//an initial ovenData where it's in bake mode and has a time of 1 second and a temp of 350
//also selector is initialized to time
OvenData ovenData = {SETUP, BAKE, 1, 1, 350, TIME, FALSE, FALSE};

// globar variables
static int16_t FRT = 0;                    //variable to work as a free running timer (incremented by 1)
static unsigned char startTime = 0;        //variable to hold the time of the FRT at the start of an event
static unsigned char cookStartTime = 0;    //variable to hold the time of the FRT at the start of an event
static unsigned char endTime = 0;          //variable to hold the time of the FRT at the end of an event
static uint8_t tickerEvent = FALSE;        //TRUE if there is a ticker event (every 5hz))


void runOvenSM(void) {
    //checks which state oven is in
    switch (ovenData.state) {
        case SETUP:
            //if there is an adc event
            if (ovenData.adcEvent) {
                //checks which mode oven is in
                switch (ovenData.mode) {
                    //for bake
                    case BAKE:
                        //checks which is selected
                        switch (ovenData.selector) {
                            case TIME:
                                //takes 8 most significant bits of adc read and add 1
                                //to create range of 1 to 256
                                ovenData.cookTime = (AdcRead() >> 2) + 1;//changes time based on adc
                                //sets remaining cook time to new time
                                ovenData.cookTimeRemaining = ovenData.cookTime;
                                break;
                            case TEMP:
                                //takes 8 most significant bits of adc read and add 1
                                //to create range of 300 to 555
                                ovenData.temperature = (AdcRead() >> 2) + 300;
                                break;
                        }
                        break;
                    case TOAST:                                          //same as BROIL
                    case BROIL:
                        //takes 8 most significant bits of adc read and add 1
                        //to create range of 1 to 256
                        ovenData.cookTime = (AdcRead() >> 2) + 1;        //changes time based on adc
                        //sets remaining cook time to new time
                        ovenData.cookTimeRemaining = ovenData.cookTime;  
                        break;
                }

            }
            //if there is a button event
            if (ovenData.buttonEvent) {
                //checks which button event happens
                switch (ovenData.buttonState) {
                    //if button 3 is down
                    case BUTTON_EVENT_3DOWN:
                        //state changes to selector change
                        ovenData.state = SELECTOR_CHANGE_PENDING;
                        //start time set to current FRT to keep track of how long button 3 is down
                        startTime = FRT;
                        break;
                    //if button 4 is down
                    case BUTTON_EVENT_4DOWN:
                        //cooking starts so start time is saved to current FRT
                        cookStartTime = FRT;
                        //state changed to cooking
                        ovenData.state = COOKING;
                        //all LEDs are lit
                        LEDS_SET(0xFF);
                        break;
                    //else nothing happens
                    default:
                        break;
                }
            }
            //update OLED display to show changes
            updateOvenOLED(ovenData);
            break;
        case SELECTOR_CHANGE_PENDING:
            //if there is a button event and it was button 3 going up
            if (ovenData.buttonEvent && ovenData.buttonState == BUTTON_EVENT_3UP) {
                //end time is current FRT since button 3 is no longer held
                endTime = FRT;
                //endTime - startTime = duration of button being held
                //if duration is greater or equal than long press
                if (endTime - startTime >= LONG_PRESS) {
                    //changes selector to the other one
                    switch (ovenData.selector) {
                        case TIME:
                            ovenData.selector = TEMP;
                            break;
                        case TEMP:
                            ovenData.selector = TIME;
                            break;
                    }
                } else {
                    //changes mode in a cycle based on what it currently is
                    switch (ovenData.mode) {
                        case BAKE:
                            ovenData.mode = TOAST;
                            break;
                        case TOAST:
                            ovenData.mode = BROIL;
                            break;
                        case BROIL:
                            ovenData.mode = BAKE;
                            break;
                    }
                }
                //changes state back to SETUP
                ovenData.state = SETUP;
            }
            //update OLED display to show changes
            updateOvenOLED(ovenData);
            break;
        case COOKING:
            //end time is current FRT
            endTime = FRT;
            //if there is a ticker event and the amount of ticks is divisible by 5
            //or every 5 ticks (1 second)
            if (tickerEvent && ((endTime - cookStartTime) % 5 == 0)) {
                //cook time remaining decremented
                ovenData.cookTimeRemaining--;
                //if cook time reaches 0
                if (ovenData.cookTimeRemaining == 0) {
                    //state changes to ALERT
                    ovenData.state = ALERT;
                    //all LEDs turned off
                    LEDS_SET(0x00);
                    //start time is now current FRT
                    startTime = FRT;
                //changes which lights are on based on the cook time remaining compared to
                //fractions of the original cook time
                } else if (ovenData.cookTimeRemaining > ((7 * ovenData.cookTime) / 8)) {
                    LEDS_SET(0xFF);
                } else if (ovenData.cookTimeRemaining > ((3 * ovenData.cookTime) / 4)) {
                    LEDS_SET(0xFE);
                } else if (ovenData.cookTimeRemaining > ((5 * ovenData.cookTime) / 8)) {
                    LEDS_SET(0xFC);
                } else if (ovenData.cookTimeRemaining > (ovenData.cookTime / 2)) {
                    LEDS_SET(0xF8);
                } else if (ovenData.cookTimeRemaining > ((3 * ovenData.cookTime) / 8)) {
                    LEDS_SET(0xF0);
                } else if (ovenData.cookTimeRemaining > (ovenData.cookTime / 4)) {
                    LEDS_SET(0xE0);
                } else if (ovenData.cookTimeRemaining > (ovenData.cookTime / 8)) {
                    LEDS_SET(0xC0);
                } else {
                    LEDS_SET(0x80);
                }
            }
            //if there is a button event and if button 4 is down
            if (ovenData.buttonEvent && ovenData.buttonState == BUTTON_EVENT_4DOWN) {
                //start time is now current FRT
                startTime = FRT;
                //state changes to reset pending
                ovenData.state = RESET_PENDING;
            }
            //update OLED display to show changes
            updateOvenOLED(ovenData);
            break;
        case RESET_PENDING:
            //if button 4 is up
            if (ovenData.buttonEvent && ovenData.buttonState == BUTTON_EVENT_4UP) {
                //end time is current FRT
                endTime = FRT;
                //duration is endTime - startTime
                //if duration is longer than long press
                if (endTime - startTime >= LONG_PRESS) {
                    //oven is reset
                    //cook time remaining becomes original cook time
                    ovenData.cookTimeRemaining = ovenData.cookTime;
                    //all LEDs turned off
                    LEDS_SET(0x00);
                    //state goes back to SETUP
                    ovenData.state = SETUP;
                } else {
                    //else, goes back to cooking
                    ovenData.state = COOKING;
                }
            }
            //update OLED display to show changes
            updateOvenOLED(ovenData);
            break;
        case ALERT:
            //end time is current FRT
            endTime = FRT;
            //if there is a ticker event and the amount of ticks is divisible by 2
            //or every 2 ticks (~0.5 seconds)
            if (tickerEvent && ((endTime - cookStartTime) % 2 == 0)) {
                //display is inverted
                OledSetDisplayInverted();
                //updates OLED
                OledUpdate();
            } else {
                //display is put back to normal
                OledSetDisplayNormal();
                //updates OLED
                OledUpdate();
            }
            //if there is a button event and button 4 is down
            if (ovenData.buttonEvent && ovenData.buttonState == BUTTON_EVENT_4DOWN) {
                    //oven is reset
                    //state goes back to set up
                    ovenData.state = SETUP;
                    //cook time remaining set to original cook time
                    ovenData.cookTimeRemaining = ovenData.cookTime;
            }
            //update OLED display to show changes
            updateOvenOLED(ovenData);
            break;
    }
}