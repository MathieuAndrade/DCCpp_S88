/**********************************************************************

  S88.cpp
  COPYRIGHT (c) 2013-2020 Xavier Bouillard & Philippe Chavatte
  Last update 2020/05/24
  http://lormedy.free.fr/S88.html
  http://lormedy.free.fr/DCCpp.html
  http://fadiese.hd.free.fr/cms/index.php?page=dcc

***********************************************************************

  DCCpp_S88 BASE STATION supports dual S88 retrosignalisation.
  This S88 Master simultanueously controls 2 S88 buses of 256 bits maxi each (32x8).
  These buses are named "Left bus" reading DataL and "Right bus" reading DataR.
  They are provided to ease operations with long layout used by clubs.
  This sensor data collection will be sent back as a binary or hexadecimal string.
  This feedback will occure as soon as any sensor change is detected or upon request of the control software.
  It will be necessary to initialize the S88 data collection indicating
  how many cards are connected (a card is a group of 8 detectors/sensors) using
  - the following variation of the "Y" command:

  <Y Nb_S88_Modules DataFormat>  with: Nb_S88_Modules to read = 2..64 (step of 2)
                                       DataFormat = 0 for binary digit answer
                                       DataFormat = 1 for hexadecimal encoded answer for CDM-Rail
                                       DataFormat = 2 for pure hexadecimal encoded answer
                                       DataFormat = 3 for SENSOR like style use by JMRI & Rocrail
                returns: <y S88status>  and <y 0001001011000000........> or <y 12C0..>
                any sensor change will send new data to the PC

  The 2 data collections have the same length : total Nb_S88_Modules = 2..64 (step of 2).
  They are concatened into 1 single buffer and send to the PC, DataL followed by DataR.

 - the following variation of the "Q" command:
  <Q>:  sets Nb_S88_Modules to read = 2..64 (step of 2)
                                       DataFormat = 2 for SENSOR like style use by JMRI & Rocrail
        returns: <Q ID> if sensor Id is active (1), <q ID> if sensor ID is inactive (0)
                 any sensor change will send new data to the PC

  Sensor list from 1 to 512 are reserved for S88 bus. Extra sensors should use ID > 512 up to 32768.
  EPRROM storage uses address 2048 to store 2 bytes
  This routine is compatible with train controller softwares CDM-Rail, WDD, CDT3x, JMRI and Rocrail.
  CDM-Rail, WDD and CDT3x are compatible with DCC++ and DCCpp, thus DCCpp_S88.

  Successfully tested with both Lormedy S88-N 8E/16E and Littfinski RM-GB-8-N modules

**********************************************************************/

#include "DCCpp.h"

#ifdef USE_S88
  #include "S88.h"
  #include "DCCpp_Uno.h"
  #include "Comm.h"

  #ifdef VISUALSTUDIO
    #include "string.h"
  #endif
  #ifdef USE_EEPROM
    #include <EEPROM.h>
    #include "EEStore.h"
  #endif
  #ifdef USE_TEXTCOMMAND
    #include "TextCommand.h"
  #endif

uint8_t  M=0;                // value read in EEPROM
uint8_t  N_size = 8;         // S88 byte size as a group of 8 sensors
uint8_t  Old_N=0;            // S88 byte number, default = 0
uint8_t  N=64;               // S88 byte number, default = 64
uint16_t Nr=0;               // S88 Bits quantity = N*8
uint8_t  DataFormat = 9;     // Output DataFormat 0=binary 1=hexa 2=Q ID; 9=disabled
uint8_t  Mode = 0;           // Output Format for extra software TBD
uint8_t  Old_DataFormat = 0; // Output DataFormat 0=binary 1=hexa
String   Old_Occ;            // S88 detector previous status
String   OccL;               // S88 detector building status
String   OccR;               // S88 detector building status
String   S88Status;          // S88 sensor status response
uint8_t  S88::S88_Cpt = 0;   // State machine position
uint8_t  dataencode = 0;

///////////////////////////////////////////////////////////////////////////////
//
long int S88::S88sampleTime = 0;
int sampleRate = 4;

boolean S88::checkTime() {
//  if (millis() - S88sampleTime < S88_SAMPLE_TIME) // no need to check S88 yet
  if (millis() - S88sampleTime < sampleRate) // no need to check S88 yet
    return (false);
  S88sampleTime = millis();                       // note millis() uses TIMER-0.
  return (true);
} // S88::checkTime

///////////////////////////////////////////////////////////////////////////////
// Acquire data on S88 bus
//

void S88::check() {
  switch (++S88_Cpt) {                            // S88 scan 512-bit >= 77ms / 13Hz
    case 1:                                       // LOAD and RESET
      digitalWrite(S88_Clock_PIN, LOW);           // Clock low
      digitalWrite(S88_Reset_PIN, LOW);           // Reset low
      digitalWrite(S88_LOAD_PS_PIN, HIGH);        // Load high 3ms min
      Nr = N/2 * N_size;                          // total bit number to read / 2
      OccL = "";
      OccR = "";
      digitalWrite(S88_Clock_PIN, HIGH);          // Clock rising ~45µs-60µs
      digitalWrite(S88_Clock_PIN, HIGH);          // Clock high
      digitalWrite(S88_Clock_PIN, HIGH);          // Clock high
      digitalWrite(S88_Clock_PIN, HIGH);          // Clock high
      digitalWrite(S88_Clock_PIN, LOW);           // Clock falling
      digitalWrite(S88_Reset_PIN, HIGH);          // Reset high 3ms min
      break;

    case 2:                                       // READ DATA stored in the last hundred of millis
      for(byte i=0 ; i<8 ; i++) {                 // read 8 sensors in a row
        OccL += String(digitalRead(S88_DataL_PIN)); // Read data, left side
        OccR += String(digitalRead(S88_DataR_PIN)); // Read data, right side
        digitalWrite(S88_LOAD_PS_PIN, LOW);       // Load low
        digitalWrite(S88_Reset_PIN, LOW);         // Reset low ~35µs
        digitalWrite(S88_Clock_PIN, HIGH);        // Clock rising ~45µs-60µs
        digitalWrite(S88_Clock_PIN, HIGH);        // Clock high
        digitalWrite(S88_Clock_PIN, HIGH);        // Clock high
        digitalWrite(S88_Clock_PIN, HIGH);        // Clock high
        digitalWrite(S88_Clock_PIN, LOW);         // Clock falling
        Nr--;
      }

      if (Nr != 0) {                              // buffers filled ?
        S88_Cpt = 1;                              // loop to case 2, ~130µs
      } else {                                    // S88 string is ready, need to format it.
        OccL += OccR;                             // concatenate 2 bus together
        if ( (digitalRead(DCCppConfig::SignalEnablePinProg) == HIGH) && ((Old_N != N) || (Old_DataFormat != DataFormat) || (Old_Occ != OccL)) ) {  // on any change: send S88 data
          if (DataFormat < 3) S88Status = "<y ";  // start of feedback (CDT3x, WDD or CDM-Rail)

          if (DataFormat == 0) {                  // binary in ASCII
            S88Status += OccL; // DCCPP_INTERFACE.println(OccL);
          } else if ( (DataFormat == 1) || (DataFormat == 2) ) {           // hexa in ASCII or pure hexa
            dataencode = 4 * DataFormat;
            for (unsigned int i = 0; i < OccL.length(); i = i + dataencode) {
              String tmp = OccL.substring(i, i + dataencode);
              int tmpint = 0;
              for (int ii = 0; ii < dataencode; ii++) {
#ifdef USE_CDMRAIL
                tmpint = tmpint | (tmp[ii] - '0') << ii;                 // lsb first
#else
                tmpint = tmpint | (tmp[ii] - '0') << (dataencode-1-ii);  // msb first
#endif
              }
              if (tmpint < 10) S88Status += tmpint;           //DCCPP_INTERFACE.print(tmpint);
              else S88Status += (char)(tmpint - 10 + 'A');    //DCCPP_INTERFACE.print((char)(tmpint+55));
            }
          } else if (DataFormat == 3) {            // JMRI, Rocrail or SENSOR style
            for (unsigned int s88index = 0; s88index < OccL.length(); s88index++) {
              String tmp = OccL.substring(s88index, s88index + 1);
              String Old_tmp = Old_Occ.substring(s88index, s88index + 1);
              if (tmp[0] != Old_tmp[0]) {
                DCCPP_INTERFACE.print( tmp[0] == '0' ? "<q " : "<Q " );
                DCCPP_INTERFACE.print(s88index+1); // s88index range 0..511
                DCCPP_INTERFACE.print(">");
#if !defined(USE_ETHERNET)
                DCCPP_INTERFACE.println("");
#endif
              }
            }
          }

          if (DataFormat < 3) {
          S88Status += ">";                        // end of feedback (CDT3x or CDM-Rail)
            DCCPP_INTERFACE.print(S88Status);      // send automaticly data at each sensors state change
#if !defined(USE_ETHERNET)
            DCCPP_INTERFACE.println("");
#endif
          }

          Old_Occ = OccL;      // save data
          Old_N = N;
          Old_DataFormat = DataFormat;
        }

        S88_Cpt = 0;           // reset to case 1
      }
      break;
    default:
      S88_Cpt = 0;             // reset to case 1
      break;
  }   // end of switch (S88_Cpt)
}     // end of S88::check

///////////////////////////////////////////////////////////////////////////////
// DCCPP_INTERFACE with CDT3x, controller, TCOWiFi, CDM-Rail, JMRI and Rocrail softwares
//
// <Y Nb_S88_Modules DataFormat> with Nb_S88_Modules=0..64 and DataFormat=0 for binary output in ASCII,
//                                                             DataFormat=1 for hexadecimal output in ASCII,
//                                                             DataFormat=2 for pure hexa,
//        returns: <y S88status>
//
// <Q>    DataFormat=3 SENSOR style JMRI & Rocrail output
//        returns: <q ID> or <Q ID>
//

#if defined(USE_TEXTCOMMAND)
void S88::parse(char *c) {
  int n, f, m;

  switch (sscanf(c, "%d %d %d", &n, &f, &m)) {
    case -1:                   // no arguments
        Old_Occ = "";
        S88_Cpt = 0;           // reset to case 1
      break;

    case 1:                    // argument is string with Nb_S88_Modules (default DataFormat is Binary)
      if (n < 0 || n > 64 || (n & 1 == 1)) {
        DCCPP_INTERFACE.print(F ("<X Bad Argument value>") );                            // Bad Argument Value

#if !defined(USE_ETHERNET)
        DCCPP_INTERFACE.println("");
#endif
      } else {
        DataFormat = (n > 0) ? 0 : 9;      // Output DataFormat 0=binAscii 9=stop
        N = n;                 // S88 byte length
        DCCPP_INTERFACE.print("<o " + String(N) + "*8 " + String(DataFormat) + ">");     // confirm command was receceived

#if !defined(USE_ETHERNET)
        DCCPP_INTERFACE.println("");
#endif

#ifdef USE_EEPROM
        if (N != M) store();
#endif
        Old_Occ = "";
        S88_Cpt = 0;           // reset to case 1 if n > 0, if 0 stop
      }
      break;

    case 2:                    // argument is string with Nb_S88_Modules and DataFormat
      if ((n < 0 || n > 64) || (n & 1 == 1) || f < 0 || f > 3) {
        DCCPP_INTERFACE.print(F ("<X Bad Argument value>") );                            // Bad Argument Value

#if !defined(USE_ETHERNET)
        DCCPP_INTERFACE.println("");
#endif
      } else {
        DataFormat = (n > 0) ? f : 9;      // Output DataFormat 0=binAscii 1=hexAscii 2=pure hexa 9=stop
        N = n;                 // S88 byte length

//        DataFormat = 3;        // JMRI, Rocrail or SENSOR style
        if (f != 3) {
          DCCPP_INTERFACE.print("<o " + String(N) + "*8 " + String(DataFormat) + ">");   // confirm command was receceived

#if !defined(USE_ETHERNET)
          DCCPP_INTERFACE.println("");
#endif
        }

#ifdef USE_EEPROM
        load();
        if (N != M) store();
#endif
        Old_Occ = "";
        S88_Cpt = 0;           // reset to case 1 if n > 0, if 0 stop
      }
      break;

    default:                   // argument count incorrect (0, 1, 2 or 3 are valid)
      DCCPP_INTERFACE.print(F ("<x Bad Argument count>") );                              // Bad Argument count

#if !defined(USE_ETHERNET)
      DCCPP_INTERFACE.println("");
#endif
      break;
  } // end of switch

  if (N > 0)
    sampleRate = 2 + 112/N;
  else
    sampleRate = 48;

}
#endif  // USE_TEXTCOMMAND

//////////////////////////////////////////////////////////////////////////////

int S88::count() {
  int count = M;               // count = 0..64 (* 8 = 0..512 sensors max)

  return count;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_EEPROM
unsigned int EEPROM_S88Address = 4088;    // address where is stored the length of your S88 bus using 2 bytes
void S88::load() {
  struct S88Data data;

  EEStore::eeAddress = EEPROM_S88Address;

#ifdef VISUALSTUDIO
  EEPROM.get(EEStore::pointer(), data, sizeof(S88Data));  // ArduiEmulator version...
#else
  EEPROM.get(EEStore::pointer(), data);
#endif

  M = data.Nn;

  EEStore::data.nS88 = M;

//  EEStore::advance(sizeof(data));

#ifdef DCCPP_DEBUG_MODE
  DCCPP_INTERFACE.print(F ("Load S88 : M ") );
  DCCPP_INTERFACE.print(M);
#if !defined(USE_ETHERNET)
  DCCPP_INTERFACE.println("");
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

void S88::store() {

  EEStore::eeAddress = EEPROM_S88Address;

#ifdef VISUALSTUDIO
  EEPROM.put(EEStore::pointer(), N, sizeof(N));  // ArduiEmulator version...
#else
  EEPROM.put(EEStore::pointer(), N);
#endif

//  EEStore::advance(sizeof(N));

#ifdef DCCPP_DEBUG_MODE
  DCCPP_INTERFACE.print(F ("Store S88 <N ") );
  DCCPP_INTERFACE.print(N);
  DCCPP_INTERFACE.print(">");
#if !defined(USE_ETHERNET)
  DCCPP_INTERFACE.println("");
#endif
#endif
}
#endif  // USE_EEPROM

#endif  // USE_S88
