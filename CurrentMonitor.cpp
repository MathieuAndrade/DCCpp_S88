#include "Config.h"
#include "DCCpp.h"
#include "CurrentMonitor.h"

long int CurrentMonitor::sampleTime = 0;

#ifdef USE_CDE_BOOSTER
static unsigned long eStopTimer = 0;
static boolean eStop_mem = false;
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

// The enable pin driven by powerOn() and powerOff(). Both districts are
// switched together, so either one tells whether the track is live.
static int enablePin()
{
    int pin = DCCppConfig::SignalEnablePinProg;
    if (pin == UNDEFINED_PIN)
    {
        pin = DCCppConfig::SignalEnablePinMain;
    }
    return pin;
}

// The emergency stop inputs belong to the station, not to one district, so
// they are sampled once per loop rather than once per monitor. Reading them
// from check() meant four analogRead() calls per pass on top of the two
// current readings, and analogRead() blocks for about 112 us: at the former
// one millisecond period that was roughly two thirds of the CPU.
void CurrentMonitor::checkSafetyInputs()
{
    bool powerState = (digitalRead(enablePin()) == HIGH);

    if (powerState && (analogRead(EmergencyStop) < 130)) // low active
    {
        DCCpp::powerOff();
        DCCPP_INTERFACE.println(F("Emergency Stop"));
        return;
    }

#ifdef USE_CDE_BOOSTER
    boolean eStop = (analogRead(E_BoosterIn) < 550); // low active, for Booster CDE

    if (eStop)
    {
        if (!eStop_mem)
        {
            // First detection: remember it and let the next pass confirm.
            eStop_mem = true;
            eStopTimer = millis();
            return;
        }
        if (millis() - eStopTimer >= 50)
        {
            // Already acted upon, nothing more to do while it stays asserted.
            return;
        }
        eStop = (analogRead(E_BoosterIn) < 550); // confirm inside the debounce window
    }

    if (eStop && powerState && eStop_mem)
    {
        DCCpp::panicStop(true);
        DCCPP_INTERFACE.println(F("E_Booster OFF"));
    }

    if (!eStop && !powerState && eStop_mem)
    {
        DCCpp::panicStop(false);
        DCCPP_INTERFACE.println(F("E_Booster ON"));
    }

    eStop_mem = eStop;
#endif // USE_CDE_BOOSTER
}

void CurrentMonitor::check()
{
    if (this->pin == UNDEFINED_PIN)
    {
        return;
    }

    this->current = (float)(analogRead(this->pin) * CURRENT_SAMPLE_SMOOTHING + this->current * (1.0 - CURRENT_SAMPLE_SMOOTHING)); // compute new exponentially-smoothed current

    // Current overload. The power state must be part of the test, otherwise
    // this fires again on every sample while the smoothed current decays,
    // flooding the interface with <p0> and stop orders.
    if (this->current > this->currentSampleMax && digitalRead(enablePin()) == HIGH)
    {
        DCCPP_INTERFACE.println(this->msg); // <p2> for main, <p4> for prog: says which district tripped
        DCCpp::powerOff();
    }
}
