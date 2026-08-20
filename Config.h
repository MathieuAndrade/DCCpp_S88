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

/** Emergency stop input coming from a CDE style booster (ROCO and similar),
low active on E_BoosterIn. Comment this line out when no CDE booster is wired:
the input then floats and triggers random power cuts. */
#define USE_CDE_BOOSTER
#define E_BoosterIn A7

// A0 reads current of "Main" power district
// A1 reads current of "Prog" power district
// A2, A3, A4 and A6 free

// Configuration for MEGA
#define S88_Clock_PIN 5 // output, Clock

#define S88_LOAD_PS_PIN 6 // output, Load request for slave devices
#define S88_Reset_PIN 7   // output, Reset request for slave devices
#define S88_DataL_PIN 8   // input,  DataL on left side
#define S88_DataR_PIN 9   // input,  DataR on right side

#endif
