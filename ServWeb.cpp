//
//  ServWeb.cpp
//
//
//  Created by Christophe on 22/11/2016.
//  Modified 25/11/2016 09:10:52
//

#include "ServWeb.h"
#include "TextCommand.h"
///#include "DCCpp_Ethernet.h"
#include "DCCpp.h"
#include "Comm.h"
#include <SD.h>


void ServWeb::sendFile(String const *file) {
  File dataFile = SD.open(*file); // Ouverture du fichier
  if (dataFile) {
    while (dataFile.available()) {
      DCCPP_INTERFACE.write(dataFile.read());
    }
  }
  else {
    DCCPP_INTERFACE.print("No file found : ");
    DCCPP_INTERFACE.println(*file);
  }
  dataFile.close();
}

void ServWeb::parse(char *c) {
  int y;
  String file;
  switch (sscanf(c, "%d", &y)) {
    case 1: // avec parametre
      switch (y) {
        case 3: // valeur du parametre == 3
          file = LOCOS_JSON;
          break;
      }
      break;
    case -1: // sans param : envoyer control.htm
      file = CONTROL_HTM;
      break;
  }
  sendFile( &file );
}
