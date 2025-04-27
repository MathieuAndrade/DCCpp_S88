//-------------------------------------------------------------------
#ifndef __DCCpp_H__
#define __DCCpp_H__

#define LIBRARY_VERSION "VERSION DCCpp library: 1.3.2"

//  Inclusion area
#define USE_S88
#define USE_TEXTCOMMAND
// #define PING_MASTER

#endif

/////////////////////////////////////

#include "DCCpp_Uno.h"
#include "PacketRegister.h"
#include "CurrentMonitor.h"
#include "Config.h"

#ifdef USE_S88
#include "S88.h"
#endif
#ifdef USE_TEXTCOMMAND
#include "TextCommand.h"
#endif

#include "DCCpp.hpp"
