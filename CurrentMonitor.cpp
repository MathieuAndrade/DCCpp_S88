/**********************************************************************

CurrentMonitor.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "DCCpp_Uno.h"
#include "CurrentMonitor.h"
#include "Comm.h"

long int eStopTimer = 0;

///////////////////////////////////////////////////////////////////////////////

void CurrentMonitor::begin(int pin, const char *msg, float inSampleMax)
{
    this->pin = pin;
    this->msg = msg;
    this->current = 0;
    this->currentSampleMax = inSampleMax;
} // CurrentMonitor::begin

boolean CurrentMonitor::checkTime()
{
    if(millis( ) - sampleTime < CURRENT_SAMPLE_TIME)       // no need to check current yet
        return(false);
    sampleTime = millis();                                 // note millis() uses TIMER-0.  For UNO, we change the scale on Timer-0.  For MEGA we do not.  This means CURENT_SAMPLE_TIME is different for UNO then MEGA
    return(true);
} // CurrentMonitor::checkTime

boolean eStop_mem = false;
void CurrentMonitor::check()
{
    if (this->pin == UNDEFINED_PIN)
        return;

    this->current = (float)(analogRead(this->pin) * CURRENT_SAMPLE_SMOOTHING + this->current * (1.0 - CURRENT_SAMPLE_SMOOTHING));      // compute new exponentially-smoothed current

    int signalPin = DCCppConfig::SignalEnablePinProg;
    if (signalPin == UNDEFINED_PIN)
        signalPin = DCCppConfig::SignalEnablePinMain;

    volatile bool powerState = (digitalRead(signalPin) == HIGH) ? true : false;
    if (powerState && (analogRead(EmergencyStop) < 130)) { // low active
        DCCpp::powerOff();
        DCCPP_INTERFACE.println(F("Emergency Stop"));
        DCCPP_INTERFACE.println(analogRead(EmergencyStop));
        #if !defined(USE_ETHERNET)
            DCCPP_INTERFACE.println("");
        #endif
    }

#ifdef E_BOOSTER_ENABLE
    boolean eStop = (analogRead(E_BoosterIn) < 550) ? true : false; // low active, for Booster CDE

    #ifdef DCCPP_DEBUG_MODE
        if (eStop != eStop_mem) {
            DCCPP_INTERFACE.print("eStop "); DCCPP_INTERFACE.print(eStop); DCCPP_INTERFACE.print("   "); DCCPP_INTERFACE.println(analogRead(E_BoosterIn));
            DCCPP_INTERFACE.print("powerState "); DCCPP_INTERFACE.println(powerState);
        }
    #endif

    if (eStop) {
        if (eStop_mem) {
            if (millis() - eStopTimer < 50) {
                eStop = (analogRead(E_BoosterIn) < 550) ? true : false;
            } else {
                goto finished;
            }
        } else {
            eStop_mem = eStop;
            eStopTimer = millis();
            goto finished;
        }
    }

    if (eStop && powerState && eStop_mem) {
        DCCpp::panicStop(eStop);
        DCCPP_INTERFACE.print(F("E_Booster OFF"));

    #if !defined(USE_ETHERNET)
        DCCPP_INTERFACE.println("");
    #endif
    }

    if (!eStop && !powerState && eStop_mem){
        DCCpp::panicStop(eStop);
        DCCPP_INTERFACE.print(F("E_Booster ON"));

    #if !defined(USE_ETHERNET)
        DCCPP_INTERFACE.println("");
    #endif
    }
    
    eStop_mem = eStop;
    finished:
#endif

    // current overload and Programming Signal is on (or could have checked Main Signal, since both are always on or off together)
    if (this->current > this->currentSampleMax && digitalRead(this->signalPin) == HIGH)
    {
        //DCCpp::powerOff();
        String origin;
        switch(this->pin) {
          case 54:
            origin = "Main";
            break;
          case 55:
            origin = "Prog";
            break;
          case 56:
            origin = "Ext";
            break;
          case 57:
            origin = "Garage/dépot";
            break;
        }

#ifdef DCCPP_DEBUG_MODE
        DCCPP_INTERFACE.print(origin);
        DCCPP_INTERFACE.println(F(" Power OFF ! Surcharge en courant détectée *****"));
        DCCPP_INTERFACE.print(F("Courant lu : "));
        DCCPP_INTERFACE.println(current);

        DCCPP_INTERFACE.print(this->msg);                  // print corresponding error message
  #if !defined(USE_ETHERNET)
        DCCPP_INTERFACE.println("");
  #endif
#endif
    }
} // CurrentMonitor::check

long int CurrentMonitor::sampleTime=0;
