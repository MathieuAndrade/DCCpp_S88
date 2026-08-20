/**********************************************************************

S88.h
COPYRIGHT (c) 2013-2020 Xavier Bouillard & Philippe Chavatte

**********************************************************************/

#ifndef S88_h
#define S88_h

#include "DCCpp.h"
#include "Arduino.h"

struct S88
{
  static long int S88sampleTime;
  static byte S88_Cpt;

  static boolean checkTime();
  static void check();
  static void parse(char *c);
};

#endif // S88_h
