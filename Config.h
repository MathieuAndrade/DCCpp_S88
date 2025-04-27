#ifndef __config_h
#define __config_h

#include "Arduino.h"

/** Use it as an argument to specify an unused pin. */
#define UNDEFINED_PIN 255

/** Number of track registers for the main line. 0 for transient orders, the others for continual orders. */
#define MAX_MAIN_REGISTERS 12
/** Number of track registers for the programming line. 0 for transient orders, the two others for continual orders for the only loco on this track. */
#define MAX_PROG_REGISTERS 3

#define DCCPP_INTERFACE Serial
#define VERSION "2.0.0"

#define DCC_SIGNAL_PIN_MAIN 12 // Arduino Mega - uses OC1B
#define DCC_SIGNAL_PIN_PROG 2  // Arduino Mega - uses OC3B

struct DCCppConfig
{
    static byte SignalEnablePinMain; // PWM : *_SIGNAL_ENABLE_PIN_MAIN
    static byte CurrentMonitorMain;  // Current sensor : *_CURRENT_MONITOR_PIN_MAIN

    static byte SignalEnablePinProg; // PWM : *_SIGNAL_ENABLE_PIN_PROG
    static byte CurrentMonitorProg;  // Current sensor : *_CURRENT_MONITOR_PIN_PROG
};

#define EmergencyStop A5
#define E_BoosterIn A7

// A0 reads current of "Main" power district
// A1 reads current of "Prog" power district
#define i_Ext A2    // A2 reads current of "External" power district
#define i_Garage A3 // A3 reads current of "Garage" power district
#define i_Depot A4  // A4 reads current of "Depot" power district
                    // A5 free
                    // A6 free

#define EN_Ext 14    // enable "External" power district
#define EN_Garage 15 // enable "Garage" power district
#define EN_Depot 16  // enable "Depot" power district

#define USE_CDMRAIL // to be defined if needed

#define S88_VERSION "VERSION S88 library:   1.3.0"

#define S88_SAMPLE_TIME 4 // ms, to be adjusted for S88 clock

// Configuration for MEGA
#define S88_Clock_PIN 5 // output, Clock

#define S88_LOAD_PS_PIN 6 // output, Load request for slave devices
#define S88_Reset_PIN 7   // output, Reset request for slave devices
#define S88_DataL_PIN 8   // input,  DataL on left side
#define S88_DataR_PIN 9   // input,  DataR on right side

#endif
