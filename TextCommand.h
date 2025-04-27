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
};

#endif
