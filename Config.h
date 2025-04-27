/**********************************************************************

Config.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman
Adapted for DcDcc by Thierry PARIS

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
#ifndef __config_h
#define __config_h

#include "Arduino.h"

/** Use it as an argument to specify an unused pin. */
#define UNDEFINED_PIN 255

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE NUMBER OF MAIN TRACK REGISTER

/** Number of track registers for the main line. 0 for transient orders, the others for continual orders. */
#define MAX_MAIN_REGISTERS 12
/** Number of track registers for the programming line. 0 for transient orders, the two others for continual orders for the only loco on this track. */
#define MAX_PROG_REGISTERS 3

#define DCCPP_INTERFACE Serial
#define VERSION "2.0.0"

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE PINS ACCORDING TO MOTOR SHIELD MODEL
//

#ifdef ARDUINO_AVR_MEGA // is using Mega 1280, define as Mega 2560 (pinouts and functionality are identical)
#define ARDUINO_AVR_MEGA2560
#define MAX_MAIN_REGISTERS 100
#endif

#define DCC_SIGNAL_PIN_MAIN 12 // Arduino Mega - uses OC1B
#define DCC_SIGNAL_PIN_PROG 2  // Arduino Mega - uses OC3B

struct DCCppConfig
{
    static byte SignalEnablePinMain; // PWM : *_SIGNAL_ENABLE_PIN_MAIN
    static byte CurrentMonitorMain;  // Current sensor : *_CURRENT_MONITOR_PIN_MAIN

    static byte SignalEnablePinProg; // PWM : *_SIGNAL_ENABLE_PIN_PROG
    static byte CurrentMonitorProg;  // Current sensor : *_CURRENT_MONITOR_PIN_PROG

    // Only for shields : indirection of the signal from SignalPinMain to DirectionMotor of the shield
    static byte DirectionMotorA; // *_DIRECTION_MOTOR_CHANNEL_PIN_A
    static byte DirectionMotorB; // *_DIRECTION_MOTOR_CHANNEL_PIN_B
};

/////////////////////////////////////////////////////////////////////////////////////

#endif
