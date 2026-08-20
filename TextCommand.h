/**********************************************************************

SerialCommand.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef TextCommand_h
#define TextCommand_h

#include "DCCpp.h"

#include "CurrentMonitor.h"
#include "PacketRegister.h"

#define MAX_COMMAND_LENGTH 30

/**
 * DCCpp station command parser
 * This class parses and processes commands received from the serial port.
 * The command format is:
 * <command> <parameter1> <parameter2> ...
 * The command is a single character, and the parameters are space-separated values.
 * The command and parameters are enclosed in angle brackets (< and >).
 * Spaces between parameters are required, but spaces elsewhere are ignored.
 * A space between the command and the first parameter is not required.
 * The command is processed by the process() method, which is called after parsing.
 */
struct TextCommand
{
  static char commandString[MAX_COMMAND_LENGTH + 1];
  static void parse(char *);
  static void process();

  /** Reads one integer from the text, skipping any leading blanks.
  Replaces sscanf() for the command arguments: the whole formatted input
  machinery of avr-libc costs more than a kilobyte of flash for what amounts
  to reading a handful of small numbers.
  @param ioText  Text to read from. Advanced past the number on success.
  @param inBase  10 for decimal arguments, 16 for the hexadecimal packet bytes.
  @param outValue  Receives the value read.
  @return true when a number was read, false when the text held none.
  */
  static bool parseNumber(const char **ioText, int inBase, int *outValue);

  /** Reads a series of blank separated integers.
  @param inText  Text to read from.
  @param outValues  Receives the values read.
  @param inMax  Size of outValues.
  @param inBase  10 for decimal arguments, 16 for hexadecimal.
  @return how many numbers were actually read, 0 if the text held none.
  */
  static int parseNumbers(const char *inText, int *outValues, int inMax, int inBase = 10);
};

#endif
