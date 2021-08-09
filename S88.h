/**********************************************************************

S88.h
COPYRIGHT (c) 2013-2020 Xavier Bouillard & Philippe Chavatte

**********************************************************************/

#ifndef S88_h
#define S88_h

#include "DCCpp.h"

#ifdef USE_S88
#include "Arduino.h"

#define USE_CDMRAIL            // to be defined if needed

#define S88_VERSION     "VERSION S88 library:   1.3.0"

#define S88_SAMPLE_TIME 4       // ms, to be adjusted for S88 clock

                                // pins number can be different if you wish
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)      // Configuration for UNO/NANO
  #define S88_Clock_PIN 2       // output, Clock
#else                                                          // Configuration for MEGA
  #define S88_Clock_PIN 5       // output, Clock
#endif

#define S88_LOAD_PS_PIN 6       // output, Load request for slave devices
#define S88_Reset_PIN   7       // output, Reset request for slave devices
#define S88_DataL_PIN   8       // input,  DataL on left side
#define S88_DataR_PIN   9       // input,  DataR on right side

struct S88Data {
  int Nn;
};

struct S88{
  struct S88Data data;  /**< Data associated to this output.*/
  static long int S88sampleTime; 
  static byte S88_Cpt;
  
  static boolean checkTime();
  
  static void check();

#if defined(USE_EEPROM)
  int eepromPos;  /**< EEPROM starting address for this S88 storage.*/

  static int count();
  /** Count the number of S88 bytes length from EEPROM.
  @remark Only available when USE_EEPROM is defined.
  */

  static void load();
  /** Load the content of S88 bytes length from EEPROM.
  @remark Only available when USE_EEPROM is defined.
  */

  static void store();
  /** Store the content of S88 bytes length in EEPROM.
  @remark Only available when USE_EEPROM is defined.
  */
#endif

#if defined(USE_TEXTCOMMAND)
  /** Do command according to the given command string. */
  static void parse(char *c);  // if different Y commands available
#endif

}; // S88

#endif  // USE_S88
#endif  // S88_h
