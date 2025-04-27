//-------------------------------------------------------------------
#ifndef __DCCpp_H__
#define __DCCpp_H__

#define LIBRARY_VERSION "VERSION DCCpp library: 1.3.2"

//  Inclusion area
#define USE_TEXTCOMMAND

#endif

/////////////////////////////////////

#include "PacketRegister.h"
#include "CurrentMonitor.h"
#include "Config.h"
#include "S88.h"

#ifdef USE_TEXTCOMMAND
#include "TextCommand.h"
#endif

#include "DCCpp.hpp"
