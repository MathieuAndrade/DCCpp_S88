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
                                       DataFormat = 3 for SENSOR like style use by JMRI & Rocrail
        returns: <Q ID> if sensor Id is active (1), <q ID> if sensor ID is inactive (0)
                 any sensor change will send new data to the PC

  Sensor list from 1 to 512 are reserved for S88 bus. Extra sensors should use ID > 512 up to 32768.
  This routine is compatible with train controller softwares CDM-Rail, WDD, CDT3x, JMRI and Rocrail.
  CDM-Rail, WDD and CDT3x are compatible with DCC++ and DCCpp, thus DCCpp_S88.

  Successfully tested with both Lormedy S88-N 8E/16E and Littfinski RM-GB-8-N modules

**********************************************************************/

#include "DCCpp.h"
#include "S88.h"
#include "TextCommand.h"

// Largest bus the <Y> command accepts: 64 modules of 8 sensors, split evenly
// between the left and the right bus, so 512 sensors and 64 bytes of buffer.
#define S88_MAX_MODULES 64
#define S88_BUFFER_BYTES (S88_MAX_MODULES * 8 / 8)

static uint8_t N_size = 8;     // sensors per module
static uint8_t N = 64;         // modules to read, both buses together, 0..64 even
static uint8_t DataFormat = 9; // 0 binary, 1 hexa CDM-Rail, 2 plain hexa, 3 sensor style, 9 disabled

// Occupancy is held one bit per sensor rather than one character per sensor.
// Bit i is sensor i + 1: the left bus fills the first half of the buffer and
// the right bus the second half, which is the order it is reported in.
static uint8_t occ[S88_BUFFER_BYTES];
static uint8_t oldOcc[S88_BUFFER_BYTES];
static bool oldValid = false; // false until oldOcc holds a comparable snapshot

static uint16_t halfBits = 0; // sensors per bus for the scan in progress
static uint16_t bitsRead = 0; // sensors already read on each bus

uint8_t S88::S88_Cpt = 0; // state machine position
long int S88::S88sampleTime = 0;
static int sampleRate = 4;

///////////////////////////////////////////////////////////////////////////////

static inline void bufWrite(uint8_t *buf, uint16_t index, uint8_t value)
{
  if (value)
    buf[index >> 3] |= (uint8_t)(1 << (index & 7));
  else
    buf[index >> 3] &= (uint8_t)(~(1 << (index & 7)));
}

static inline uint8_t bufRead(const uint8_t *buf, uint16_t index)
{
  return (uint8_t)((buf[index >> 3] >> (index & 7)) & 1);
}

///////////////////////////////////////////////////////////////////////////////

boolean S88::checkTime()
{
  if (millis() - (unsigned long)S88sampleTime < (unsigned long)sampleRate)
    return (false);
  S88sampleTime = millis(); // note millis() uses TIMER-0.
  return (true);
} // S88::checkTime

///////////////////////////////////////////////////////////////////////////////
// Report the occupancy buffer, but only when it changed and the track is on.

static void sendFeedback()
{
  uint16_t totalBits = halfBits * 2;

  if (digitalRead(DCCppConfig::SignalEnablePinProg) != HIGH)
    return;

  if (oldValid && memcmp(occ, oldOcc, (size_t)((totalBits + 7) / 8)) == 0)
    return;

  if (DataFormat == 3)
  {
    // JMRI and Rocrail style: one message per sensor that changed.
    for (uint16_t i = 0; i < totalBits; i++)
    {
      uint8_t now = bufRead(occ, i);
      if (!oldValid || now != bufRead(oldOcc, i))
      {
        DCCPP_INTERFACE.print(now ? "<Q " : "<q ");
        DCCPP_INTERFACE.print(i + 1); // sensor ids run from 1
        DCCPP_INTERFACE.println(">");
      }
    }
  }
  else
  {
    DCCPP_INTERFACE.print("<y ");

    if (DataFormat == 0)
    {
      for (uint16_t i = 0; i < totalBits; i++)
        DCCPP_INTERFACE.print(bufRead(occ, i) ? '1' : '0');
    }
    else
    {
      // One hexadecimal digit per group of four sensors. DataFormat 1 is the
      // CDM-Rail order, where the first sensor of the group is the least
      // significant bit; DataFormat 2 is plain hexadecimal, most significant
      // bit first. That choice used to be a compile time switch, while
      // DataFormat 2 doubled the group size instead and emitted characters
      // outside the hexadecimal range.
      for (uint16_t i = 0; i < totalBits; i += 4)
      {
        uint8_t nibble = 0;
        for (uint8_t b = 0; b < 4; b++)
        {
          uint8_t shift = (DataFormat == 1) ? b : (uint8_t)(3 - b);
          nibble |= (uint8_t)(bufRead(occ, i + b) << shift);
        }
        DCCPP_INTERFACE.print(nibble, HEX);
      }
    }

    DCCPP_INTERFACE.println(">");
  }

  memcpy(oldOcc, occ, sizeof(oldOcc));
  oldValid = true;
} // sendFeedback

///////////////////////////////////////////////////////////////////////////////
// Acquire data on S88 bus

void S88::check()
{
  // Nothing to scan while the bus is disabled or declared empty. Without this
  // the state machine still ran with N == 0: the bit counter was 0, the read
  // loop decremented it eight times anyway and wrapped the unsigned counter to
  // 65528, so a <Y 0> spent thousands of iterations clocking the bus for
  // nothing while the buffers kept growing.
  if (N == 0 || DataFormat > 3)
    return;

  switch (++S88_Cpt)
  {
  case 1:                                // LOAD and RESET
    digitalWrite(S88_Clock_PIN, LOW);    // Clock low
    digitalWrite(S88_Reset_PIN, LOW);    // Reset low
    digitalWrite(S88_LOAD_PS_PIN, HIGH); // Load high 3ms min
    halfBits = (uint16_t)(N / 2) * N_size;
    bitsRead = 0;
    digitalWrite(S88_Clock_PIN, HIGH); // Clock rising ~45us-60us
    digitalWrite(S88_Clock_PIN, HIGH); // Clock high
    digitalWrite(S88_Clock_PIN, HIGH); // Clock high
    digitalWrite(S88_Clock_PIN, HIGH); // Clock high
    digitalWrite(S88_Clock_PIN, LOW);  // Clock falling
    digitalWrite(S88_Reset_PIN, HIGH); // Reset high 3ms min
    break;

  case 2: // READ DATA stored in the last hundred of millis
    // The bitsRead test bounds the loop as well, so the counter can never run
    // past the end of the buffer whatever N holds.
    for (byte i = 0; i < 8 && bitsRead < halfBits; i++)
    {
      bufWrite(occ, bitsRead, digitalRead(S88_DataL_PIN));            // Read data, left side
      bufWrite(occ, halfBits + bitsRead, digitalRead(S88_DataR_PIN)); // Read data, right side
      digitalWrite(S88_LOAD_PS_PIN, LOW);                             // Load low
      digitalWrite(S88_Reset_PIN, LOW);                               // Reset low ~35us
      digitalWrite(S88_Clock_PIN, HIGH);                              // Clock rising ~45us-60us
      digitalWrite(S88_Clock_PIN, HIGH);                              // Clock high
      digitalWrite(S88_Clock_PIN, HIGH);                              // Clock high
      digitalWrite(S88_Clock_PIN, HIGH);                              // Clock high
      digitalWrite(S88_Clock_PIN, LOW);                               // Clock falling
      bitsRead++;
    }

    if (bitsRead < halfBits)
    {
      S88_Cpt = 1; // buffer not filled yet, read another group of eight
    }
    else
    {
      sendFeedback();
      S88_Cpt = 0; // reset to case 1
    }
    break;

  default:
    S88_Cpt = 0; // reset to case 1
    break;
  } // end of switch (S88_Cpt)
} // end of S88::check

///////////////////////////////////////////////////////////////////////////////
// DCCPP_INTERFACE with CDT3x, controller, TCOWiFi, CDM-Rail, JMRI and Rocrail
//
// <Y Nb_S88_Modules DataFormat> with Nb_S88_Modules = 0..64 (even) and
//                               DataFormat = 0 binary in ASCII,
//                                            1 hexadecimal for CDM-Rail,
//                                            2 plain hexadecimal,
//                                            3 sensor style for JMRI/Rocrail
//        returns: <o modules*8 format> then <y S88status>
//
// <Q>    DataFormat = 3, sensor style output
//        returns: <q ID> or <Q ID>

void S88::parse(char *c)
{
  int n, f, m;

  switch (sscanf(c, "%d %d %d", &n, &f, &m))
  {
  case -1: // no arguments: ask for a full refresh
    oldValid = false;
    S88_Cpt = 0; // reset to case 1
    break;

  case 1: // argument is string with Nb_S88_Modules (default DataFormat is Binary)
    if (n < 0 || n > S88_MAX_MODULES || ((n & 1) == 1))
    {
      DCCPP_INTERFACE.println(F("<X Bad Argument value>")); // Bad Argument Value
    }
    else
    {
      DataFormat = (n > 0) ? 0 : 9; // Output DataFormat 0=binAscii 9=stop
      N = (uint8_t)n;               // S88 byte length
      DCCPP_INTERFACE.println("<o " + String(N) + "*8 " + String(DataFormat) + ">");

      oldValid = false;
      S88_Cpt = 0; // reset to case 1 if n > 0, if 0 stop
    }
    break;

  case 2: // argument is string with Nb_S88_Modules and DataFormat
    if (n < 0 || n > S88_MAX_MODULES || ((n & 1) == 1) || f < 0 || f > 3)
    {
      DCCPP_INTERFACE.println(F("<X Bad Argument value>")); // Bad Argument Value
    }
    else
    {
      DataFormat = (n > 0) ? (uint8_t)f : 9; // 0=binAscii 1=hexAscii 2=plain hexa 3=sensor 9=stop
      N = (uint8_t)n;                        // S88 byte length

      if (f != 3)
      {
        DCCPP_INTERFACE.println("<o " + String(N) + "*8 " + String(DataFormat) + ">");
      }

      oldValid = false;
      S88_Cpt = 0; // reset to case 1 if n > 0, if 0 stop
    }
    break;

  default:                                                // argument count incorrect (0, 1 or 2 are valid)
    DCCPP_INTERFACE.println(F("<x Bad Argument count>")); // Bad Argument count
    break;
  } // end of switch

  if (N > 0)
    sampleRate = 2 + 112 / N;
  else
    sampleRate = 48;
} // S88::parse
