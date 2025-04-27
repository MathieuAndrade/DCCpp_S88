/**********************************************************************

S88.h
COPYRIGHT (c) 2013-2020 Xavier Bouillard & Philippe Chavatte

**********************************************************************/

#ifndef S88_h
#define S88_h

#include "DCCpp.h"
#include "Arduino.h"

#define USE_CDMRAIL // to be defined if needed

#define S88_VERSION "VERSION S88 library:   1.3.0"

#define S88_SAMPLE_TIME 4 // ms, to be adjusted for S88 clock

// Configuration for MEGA
#define S88_Clock_PIN 5 // output, Clock

#define S88_LOAD_PS_PIN 6 // output, Load request for slave devices
#define S88_Reset_PIN 7   // output, Reset request for slave devices
#define S88_DataL_PIN 8   // input,  DataL on left side
#define S88_DataR_PIN 9   // input,  DataR on right side

struct S88Data
{
  int Nn;
};

struct S88
{
  struct S88Data data; /**< Data associated to this output.*/
  static long int S88sampleTime;
  static byte S88_Cpt;

  static boolean checkTime();

  static void check();

  /** Do command according to the given command string. */
  static void parse(char *c); // if different Y commands available

}; // S88

#endif // S88_h
