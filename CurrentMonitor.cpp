#include "Config.h"
#include "DCCpp.h"
#include "CurrentMonitor.h"

long int CurrentMonitor::sampleTime = 0;

#ifdef USE_CDE_BOOSTER
long int eStopTimer = 0;
boolean eStop_mem = false;
#endif

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

#ifdef USE_CDE_BOOSTER
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
        DCCPP_INTERFACE.println(F("E_Booster OFF"));
    }

    if (!eStop && !powerState && eStop_mem)
    {
        DCCpp::panicStop(eStop);
        DCCPP_INTERFACE.println(F("E_Booster ON"));
    }

    eStop_mem = eStop;
finished:
#endif // USE_CDE_BOOSTER

    // Current overload. The power state must be part of the test, otherwise
    // this fires again on every sample while the smoothed current decays,
    // flooding the interface with <p0> and stop orders. signalPin is the
    // local computed above: the member of the same name was never assigned,
    // so it read as pin 0 (RX0) and the guard did not hold.
    if (this->current > this->currentSampleMax && digitalRead(signalPin) == HIGH)
    {
        DCCpp::powerOff();
    }
}
