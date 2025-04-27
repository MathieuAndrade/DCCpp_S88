#include "Config.h"
#include "DCCpp.h"
#include "CurrentMonitor.h"

long int CurrentMonitor::sampleTime = 0;

long int eStopTimer = 0;

void CurrentMonitor::begin(int pin, const char *msg, float inSampleMax)
{
    this->pin = pin;
    this->msg = msg;
    this->current = 0;
    this->currentSampleMax = inSampleMax;
}

boolean CurrentMonitor::checkTime()
{
    if (millis() - sampleTime < CURRENT_SAMPLE_TIME)
    {
        return (false);
    }
    sampleTime = millis();
    return (true);
}

boolean eStop_mem = false;
void CurrentMonitor::check()
{
    if (this->pin == UNDEFINED_PIN)
    {
        return;
    }

    this->current = (float)(analogRead(this->pin) * CURRENT_SAMPLE_SMOOTHING + this->current * (1.0 - CURRENT_SAMPLE_SMOOTHING)); // compute new exponentially-smoothed current

    int signalPin = DCCppConfig::SignalEnablePinProg;
    if (signalPin == UNDEFINED_PIN)
    {
        signalPin = DCCppConfig::SignalEnablePinMain;
    }

    volatile bool powerState = (digitalRead(signalPin) == HIGH) ? true : false;

    if (powerState && (analogRead(EmergencyStop) < 130))
    { // low active
        DCCpp::powerOff();
        DCCPP_INTERFACE.println(F("Emergency Stop"));
        DCCPP_INTERFACE.println(analogRead(EmergencyStop));
    }

    boolean eStop = (analogRead(E_BoosterIn) < 550) ? true : false; // low active, for Booster CDE

    if (eStop)
    {
        if (eStop_mem)
        {
            if (millis() - eStopTimer < 50)
            {
                eStop = (analogRead(E_BoosterIn) < 550) ? true : false;
            }
            else
            {
                goto finished;
            }
        }
        else
        {
            eStop_mem = eStop;
            eStopTimer = millis();
            goto finished;
        }
    }

    if (eStop && powerState && eStop_mem)
    {
        DCCpp::panicStop(eStop);
        DCCPP_INTERFACE.print(F("E_Booster OFF"));
    }

    if (!eStop && !powerState && eStop_mem)
    {
        DCCpp::panicStop(eStop);
        DCCPP_INTERFACE.print(F("E_Booster ON"));
    }

    eStop_mem = eStop;
finished:

    // current overload and Programming Signal is on (or could have checked Main Signal, since both are always on or off together)
    if (this->current > this->currentSampleMax && digitalRead(this->signalPin) == HIGH)
    {
        // DCCpp::powerOff();
        String origin;
        switch (this->pin)
        {
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
    }
}
