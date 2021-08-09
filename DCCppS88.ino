/*************************************************************
project: <Générateur DCCpp_S88> V1.3.0
author:  <Philippe Chavatte>
description: <Serial/Ethernet DCC Generator with S88 Interface>
Last update: 31 octobre 2020

- Resultat de compilation avec Arduino MEGA :
COMM série:
Le croquis utilise 23750 octets (9%) de l'espace de stockage de programmes. Le maximum est de 253952 octets.
Les variables globales utilisent 1306 octets (15%) de mémoire dynamique, ce qui laisse 6886 octets pour les variables locales. Le maximum est de 8192 octets.

COMM Ethernet:
Le croquis utilise 39180 octets (15%) de l'espace de stockage de programmes. Le maximum est de 253952 octets.
Les variables globales utilisent 1787 octets (21%) de mémoire dynamique, ce qui laisse 6405 octets pour les variables locales. Le maximum est de 8192 octets.

*****====================== IMPORTANT ======================*****

***** Prière de sélectionner les paramétres, lignes 125-139 *****
***** Please select your setting, lines 125-139             *****
***** Ethernet info at line 169 & 177                       *****
***** More S88 infos at line 240 & 247                      *****
***** For CDE booster, see currentMonitor.h, lines 50-51    *****
 
*****=======================================================*****

- Logiciel pour générer des trames DCC à partir de commandes reçues par la liaison USB (port COM virtuel) ou Ethernet
- Compatible avec ATmega 328P
- Serialport avec Rx en 0 et Tx en 1
- Permet de piloter un réseau principal et une voie de programmation en même temps avec 2 Boosters L298N + 1 x AM26C31 ou LMD18200
- Gère 2 bus de retro-signalisation S88 de 256 capteurs maxi chacun, symbolisés par 512 bits
- Les capteurs du bus S88 Gauche sont contenu dans la première moitié des données et les capteurs du bus S88 Droit se trouvent dans la deuxième moitié des données
- Exemple : si on utilise seulement 64 capteurs (8*8), les 32 capteurs du bus Gauche sont classés de 1 à 32 et les 32 capteurs du bus Droit sont classés de 33 à 64

- Arduino MEGA :
- Main output PWM en 12, output Enable en 3 et Current Sensor input en A0
- Prog output PWM en 2, output Enable en 11 et Current Sensor input en A1
-
- Arduino NANO/UNO :
- Main output PWM en 10, output Enable en 3 et Current Sensor input en A0
- Prog output PWM en 5, output Enable en 11 et Current Sensor input en A1
-
- Extension MEGA2560 sans Ethernet shield possible avec :
- Main_ext output PWM en 12, output Enable_ext en 14,     Current Sensor_ext input en A2
- Main_acc output PWM en 12, output Enable_garage en 15,  Current Sensor_garage input en A3
- Main_acc output PWM en 12, output Enable_dépot en 16,   Current Sensor_dépot input en A4
- Main_acc output PWM en 12, output Enable_accessoire ON, No Current Sensor_accessoire input
- PanicStopButton input pin A5
- Free Pin : A6
- E_in from ROCO Booster (CDE style) input pin A7
-
- Modification des fichiers :
- DCCpp.h               lignes ajoutées 363, 446-448 (S88), décommentées 347, 353, 359, 363, 367, 368
- DCCpp.cpp             lignes ajoutées 112-117 (S88)
- Config.h              ligne modifiée 34       (#define MAX_MAIN_REGISTERS 100 au lieu de 12) pour Arduino MEGA !
- CurrentMonitor.cpp    lignes ajoutées 47-49   (print)
- TextCommand.cpp       lignes ajoutées 68, 72  (print), 95-112 (S88)
- S88.h & S88.cpp       fichiers ajoutés pour gérer la rétro-signalisation S88

*************************************************************/
/*
Pinout is defined in config.h as it follows
NANO/UNO :
        OC1B output     Pin 10 <---> PWM Main
             output     Pin 3  <---> ENA Main
        OC0B output     Pin 5  <---> PWM Prog
             output     Pin 11 <---> ENA Prog
MEGA :
        OC1B output     Pin 12 <---> PWM Main
             output     Pin 3  <---> ENA Main
        OC3B output     Pin 2  <---> PWM Prog
             output     Pin 11 <---> ENA Prog

    "NANO/UNO"                      "MEGA"
===============================================================================================
0   SERIAL_RX0                      SERIAL_RX0
1   SERIAL_TX0                      SERIAL_TX0
2   S88_Clock_PIN                   DCC_SIGNAL_PIN_PROG
3   DCC_ENABLE_PIN_MAIN             DCC_ENABLE_PIN_MAIN
4   SDCARD_CS                       SDCARD_CS
5   DCC_SIGNAL_PIN_PROG             S88_Clock_PIN
6   S88_LOAD_PS_PIN                 S88_LOAD_PS_PIN
7   S88_Reset_PIN                   S88_Reset_PIN
8   S88_DataL_PIN                   S88_DataL_PIN
9   S88_DataR_PIN                   S88_DataR_PIN
10  DCC_SIGNAL_PIN_MAIN             ------------------------------
11  DCC_ENABLE_PIN_PROG             DCC_ENABLE_PIN_PROG
12  ------------------------------  DCC_SIGNAL_PIN_MAIN
13  LED---------------------------  LED---------------------------

20                                  I2C_SDA_PIN
21                                  I2C_SCL_PIN

50                                  SPI_MISO_PIN
51                                  SPI_MOSI_PIN
52                                  SPI_SCK_PIN
53                                  SPI_SS_PIN                       Ethernet_CS

Alternative 1
A0  CURRENT_MONITOR_PIN_MAIN        CURRENT_MONITOR_PIN_MAIN
A1  CURRENT_MONITOR_PIN_PROG        CURRENT_MONITOR_PIN_PROG
A2
A3
A4  I2C_SDA_PIN
A5  I2C_SCL_PIN

Alternative 2
A0  CURRENT_MONITOR_PIN_MAIN        CURRENT_MONITOR_PIN_MAIN        voie int
A1  CURRENT_MONITOR_PIN_PROG        CURRENT_MONITOR_PIN_PROG        voie H + accessoires
A2  CURRENT_MONITOR_PIN_MAIN2       CURRENT_MONITOR_PIN_MAIN2       voie ext
A3  CURRENT_MONITOR_PIN_MAIN3       CURRENT_MONITOR_PIN_MAIN3       dépot + garage
A4  I2C_SDA_PIN                     reserved
A5  I2C_SCL_PIN                     EmergencyStop                   Stop == 0, Normal == 1
A6  unused                          unused
A7  E_BoosterIn                     E_BoosterIn                     Stop == 0, Normal == 1

**************************************************************/

/////////////////////////////////////////////////////////////////////////////////////
// DCCPP_DEBUG_MODE is defined in DCCpp.h line 354, 360, 366
/////////////////////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "DCCpp.h"
#include "Config.h"

// ***** Pour utiliser CDM-Rail avec le bus S88, dé-commenter la ligne 16 de S88.h *****

// ***** Pour activer le bouton d'arret d'urgence sur A5, placer un bouton poussoir connecté avec la masse
#define EmergencyStop A5              // also defined in CurrentMonitor.h line 52
#ifdef ARDUINO_AVR_MEGA
  #define E_BoosterIn   A7            // also defined in CurrentMonitor.h line 50-51
#endif

/////////////////////////////////////////////////////////////////////////////////////
// COMM SETUP - ***** Please select a COMM type and an Ethernet interface if needed *****
/////////////////////////////////////////////////////////////////////////////////////

#define COMM_TYPE 0                 // Serial (USB) & NANO or UNO or MEGA
// #define COMM_TYPE 1                 // Ethernet & MEGA only ==> ***** you must choose an interface in DCCpp.h line 376-379 and an IP address line 149 *****
//                                                               ***** selectionner une interface dans DCCpp.h ligne 376-379 et une adresse IP ligne 149 *****

#if COMM_TYPE == 0
  // enable serial communication
  #ifndef USE_TEXTCOMMAND
    #error To be able to compile this sample,the line #define USE_TEXTCOMMAND must be uncommented in DCCpp.h
#endif

#elif COMM_TYPE == 1
  #include "ServWeb.h"
  #include <SD.h>
  #include <SPI.h>

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE STATIC IP ADDRESS *OR* COMMENT OUT TO USE DHCP
//
// enable ethernet communication
// Circuit: ETHERNET WIZNET 5100 selected in DCCpp.h line 376
// Ethernet shield attached to pins 10, 50, 51, 52 on MEGA
/////////////////////////////////////////////////////////////////////////////////////

  // enable Ethernet communication

  #if !defined(USE_TEXTCOMMAND) || !defined(USE_ETHERNET)
    #error To be able to compile this sample, the lines #define USE_TEXTCOMMAND and #define USE_ETHERNET must be uncommented in DCCpp.h
  #endif

  //#define DCCPP_INTERFACE eServer       // defined in DCCpp_Uno.h line28

  //#define USE_ETHERNET_WIZNET_5100 and #define USE_ETHERNET // to be defined in DCCpp.h line 376

  // the media access control (Ethernet hardware) address for the shield:
  uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  //static IP address for the shield:
  uint8_t ip[] = { 192, 168, 0, 200 };

// #define USE_DHCP TRUE
  #define EthernetPort   2560      // Warning: EthernetPort must be declared in DCCpp.cpp line 347
  EthernetServer DCCPP_INTERFACE(EthernetPort); // Create and instance of an EthernetServer

  // Shields:
  // SDCARD shield attached to pins 4, 50, 51, 52 on MEGA
  //#define SDCARD_CS 4             // Reserved pin for SD card CS : 4 - defined in DCCpp_Uno.h
  #define SS 53               // 53, MOSI on MEGA. SPI doesn't work without this pin set to output !
#endif


/////////////////////////////////////////////////////////////////////////////////////
// Analog
/////////////////////////////////////////////////////////////////////////////////////

                                    // A0 reads current of "Main" power district
                                    // A1 reads current of "Prog" power district
#define i_Ext       A2              // A2 reads current of "External" power district
#define i_Garage    A3              // A3 reads current of "Garage" power district
#define i_Depot     A4              // A4 reads current of "Depot" power district
                                    // A5 free
                                    // A6 free


/////////////////////////////////////////////////////////////////////////////////////
// Digital
/////////////////////////////////////////////////////////////////////////////////////

#define EN_Ext      14              // enable "External" power district
#define EN_Garage   15              // enable "Garage" power district
#define EN_Depot    16              // enable "Depot" power district


/////////////////////////////////////////////////////////////////////////////////////
// ARDUINO SETUP
/////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  Serial.flush(); delay(1000);
  Serial.println(F("Initialisation de la liaison serie USB2COM 115200 baud"));
  Serial.println(F("Programme de conduite DCCpp_S88 pour Arduino UNO/MEGA2560 avec booster L298N et MAX471"));
  Serial.println(F("Adaptation par Philippe Chavatte - 31 juillet 2018 - lormedy.free.fr"));
  Serial.println(F("Option S88 par Philippe Chavatte - 25 mai 2019 - lormedy@free.fr")); delay(500);
  Serial.print(F(LIBRARY_VERSION)); Serial.println(F(" + S88"));
  Serial.println(F("Compatible avec CDM_Rail, WDD, TCOwifi, DMC, CDT, JMRI et Rocrail"));
  Serial.print(F("--------- Module DCC initialisé avec "));
  Serial.print(COMM_TYPE ? "Ethernet" : "Serial port @ 115200 bauds");
  Serial.println(F(" ---------")); delay(500);

  pinMode(4, INPUT_PULLUP);         // disable SD card until initialisation
  pinMode(10, INPUT_PULLUP);        // disable Ethernet until initialisation
  pinMode(EmergencyStop, INPUT_PULLUP);

#if defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_NANO)  // if needed, E_BOOSTER_ENABLE must be defined in currentMonitor.h at line 51
  pinMode(E_BoosterIn, INPUT);
#endif
/*
  pinMode(EN_Ext, OUTPUT);
  digitalWrite(EN_Ext, LOW);        // disable "External" power district
  pinMode(EN_Garage, OUTPUT);
  digitalWrite(EN_Garage, LOW);     // disable "Garage" power district
  pinMode(EN_Depot, OUTPUT);
  digitalWrite(EN_Depot, LOW);      // disable "Depot" power district
*/
// S88 setup
// ***** WARNING: using S88 disables SENSOR routine *****
// command described in S88.cpp line 22
// #define USE_S88    done in DCCpp.h line 367, can be disabled over there if unused
// S88 pins are defined in S88.h . You can change any pin number to fit your need.
// If you don't use DataR, you read all 0 on the second half of the data dump

#ifdef USE_S88                              // Les pins sont déclarées dans S88.h
  pinMode(S88_LOAD_PS_PIN, OUTPUT);         // THIS ARDUINO OUPUT PIN MUST BE PHYSICALLY CONNECTED TO THE S88 Load PIN
  pinMode(S88_Reset_PIN, OUTPUT);           // THIS ARDUINO OUPUT PIN MUST BE PHYSICALLY CONNECTED TO THE S88 Reset PIN
  pinMode(S88_Clock_PIN, OUTPUT);           // THIS ARDUINO OUPUT PIN MUST BE PHYSICALLY CONNECTED TO THE S88 Clock PIN
  pinMode(S88_DataL_PIN, INPUT);            // THIS ARDUINO OUPUT PIN MUST BE PHYSICALLY CONNECTED TO THE S88 DataL PIN (1st data in the buffer) with 10k pulldown on your board
  pinMode(S88_DataR_PIN, INPUT);            // THIS ARDUINO OUPUT PIN MUST BE PHYSICALLY CONNECTED TO THE S88 DataR PIN (last data in the buffer) with 10k pulldown on your board

  digitalWrite(S88_LOAD_PS_PIN, LOW);
  digitalWrite(S88_Clock_PIN, LOW);
  digitalWrite(S88_Reset_PIN, LOW);
#endif

#if defined(USE_ETHERNET)
  pinMode(SS, OUTPUT);              // SS for Ethernet controller
  pinMode(SDCARD_CS, OUTPUT);       // CS for SD card
  digitalWrite(SDCARD_CS, HIGH);    // disable CS
  Serial.println(F ("*************\nInitialisation SDCARD ...") );
  if (!SD.begin(SDCARD_CS)){        //teste la communication avec la carte(pin 4)
      Serial.println(F ("Communication impossible avec SDCARD") );
  }
  else {
    Serial.println(F ("Communication with SDCARD ok !") );
  }

  // You can use Ethernet.init(pin) to configure the CS pin
  // Ethernet.init(10);  // Most Arduino shields
  // Ethernet.init(5);   // MKR ETH shield

#ifdef USE_DHCP
  DCCpp::beginEthernet(mac);        // Start networking using DHCP to get an IP Address
#else
  DCCpp::beginEthernet(mac, ip);    // Start networking using a fixed IP Address
#endif

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("");
    Serial.println(F ("Ethernet shield was not found. Please use Serial.") );
  }

  else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println(F ("Ethernet cable is not connected.") );
  }
  else {
    Serial.print("Server is at ");
    Serial.println(Ethernet.localIP());
  }
#endif

  DCCpp::begin();
  // Configuration for Arduino Mega2560 + 2 L298 + 2 MAX471. See the page 'Configuration lines' in the documentation in DCCpp.h for other samples.

  DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 3, A0);        //defined in config.h : DCC_SIGNAL_PIN_MAIN 12  // Arduino Mega - uses OC1B
  //                                                                                                              10 for Arduino UNO  - uses OC1B
  DCCpp::beginProg(UNDEFINED_PIN, DCC_SIGNAL_PIN_PROG, 11, A1);       //defined in config.h : DCC_SIGNAL_PIN_PROG  2  // Arduino Mega - uses OC3B
  //                                                                                                               5 for Arduino UNO  - uses OC0B
}


/////////////////////////////////////////////////////////////////////////////////////
// MAIN ARDUINO LOOP
/////////////////////////////////////////////////////////////////////////////////////

void loop(){

// Tout le décodage des commandes reçues par la liaison série sont décodées dans le fichier "TextCommand.cpp" line 34
// L'esssentiel du travail de la boucle s'effectue dans le fichier "DCCpp.cpp" line 92
// Pour changer le nombre de locos, modifier la ligne 23 dans Config.h - "#define MAX_MAIN_REGISTERS 99"
// Pour changer la valeur du courant limite, modifier la ligne 38 dans CurrentMonitor.h - "  void begin(int pin, const char *msg, float inSampleMax = 300);"
// Valeurs : 300 ==> 1.5 A, 400 ==> 2.0 A, 500 ==> 2.5 A
// Modif pour DEBUG dans le fichier DCCpp.h line 347, 353, 359

  DCCpp::loop();    // dans DCCpp.cpp ligne 92
}
