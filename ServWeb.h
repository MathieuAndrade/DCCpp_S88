//
//  ServWeb.h
//  
//
//  Created by Christophe on 22/11/2016.
//
//

#include "Arduino.h"

#ifndef _ServWeb_h
#define _ServWeb_h

#define CONTROL_HTM   "control.htm"
#define LOCOS_JSON    "locos.jso" 
//#define CONTROL_CSS   "control.css" // For future upgrade 
//#define CONTROL_JS    "control.js"  // For future upgrade

struct ServWeb{
  static void parse(char *c);
  static void sendFile( String const *);
};

#endif
