#include "DCCpp.h"
#include "Arduino.h"

volatile RegisterList DCCpp::mainRegs(MAX_MAIN_REGISTERS);
volatile RegisterList DCCpp::progRegs(3);

CurrentMonitor DCCpp::mainMonitor;
CurrentMonitor DCCpp::progMonitor;

bool DCCpp::programMode;
bool DCCpp::panicStopped;

// *********************************************************** FunctionsState

FunctionsState::FunctionsState()
{
    this->clear();
}

void FunctionsState::clear()
{
    // Clear all functions
    this->activeFlags[0] = 0;
    this->activeFlags[1] = 0;
    this->activeFlags[2] = 0;
    this->activeFlags[3] = 0;

    this->statesSent();
}

void FunctionsState::activate(byte inFunctionNumber)
{
    bitSet(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
}

void FunctionsState::inactivate(byte inFunctionNumber)
{
    bitClear(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
}

bool FunctionsState::isActivated(byte inFunctionNumber)
{
    return bitRead(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
}

bool FunctionsState::isActivationChanged(byte inFunctionNumber)
{
    return bitRead(this->activeFlagsSent[inFunctionNumber / 8], inFunctionNumber % 8) != isActivated(inFunctionNumber);
}

void FunctionsState::statesSent()
{
    for (int i = 0; i < 4; i++)
        this->activeFlagsSent[i] = this->activeFlags[i];
}

// *********************************************************** DCCpp class

static bool first = true;
bool DCCpp::pingSend = false;
long DCCpp::pingTime = 0;
long DCCpp::pingTimeout = 4000;

void DCCpp::loop()
{
    TextCommand::process(); // check for, and process, and new serial commands

    if (first)
    {
        first = false;
    }

    if (CurrentMonitor::checkTime())
    {
        mainMonitor.check();
        progMonitor.check();
    }

    if (S88::checkTime())
    {
        S88::check();
    }
}

void DCCpp::beginMain(uint8_t inOptionalDirectionMotor, uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
    DCCppConfig::SignalEnablePinMain = inSignalEnable; // PWM
    DCCppConfig::CurrentMonitorMain = inCurrentMonitor;

    // If no main line, exit.
    if (DCCppConfig::SignalEnablePinMain == UNDEFINED_PIN)
    {
        return;
    }

    mainMonitor.begin(DCCppConfig::CurrentMonitorMain, (char *)"<p2>");

    // CONFIGURE TIMER_1 TO OUTPUT 50% DUTY CYCLE DCC SIGNALS ON OC1B INTERRUPT PINS

    // Direction Pin for Motor Shield Channel A - MAIN OPERATIONS TRACK
    // Controlled by Arduino 16-bit TIMER 1 / OC1B Interrupt Pin
    // Values for 16-bit OCR1A and OCR1B registers calibrated for 1:1 prescale at 16 MHz clock frequency
    // Resulting waveforms are 200 microseconds for a ZERO bit and 116 microseconds for a ONE bit with exactly 50% duty cycle

#define DCC_ZERO_BIT_TOTAL_DURATION_TIMER1 3199
#define DCC_ZERO_BIT_PULSE_DURATION_TIMER1 1599

#define DCC_ONE_BIT_TOTAL_DURATION_TIMER1 1855
#define DCC_ONE_BIT_PULSE_DURATION_TIMER1 927

    if (inSignalPin != UNDEFINED_PIN)
        pinMode(inSignalPin, OUTPUT); // FOR SHIELDS, THIS ARDUINO OUTPUT PIN MUST BE PHYSICALLY CONNECTED TO THE PIN FOR DIRECTION-A OF MOTOR CHANNEL-A

    bitSet(TCCR1A, WGM10); // set Timer 1 to FAST PWM, with TOP=OCR1A
    bitSet(TCCR1A, WGM11);
    bitSet(TCCR1B, WGM12);
    bitSet(TCCR1B, WGM13);

    bitSet(TCCR1A, COM1B1); // set Timer 1, OC1B (pin 12/MEGA) to inverting toggle (actual direction is arbitrary)
    bitSet(TCCR1A, COM1B0);

    bitClear(TCCR1B, CS12); // set Timer 1 prescale=1
    bitClear(TCCR1B, CS11);
    bitSet(TCCR1B, CS10);

    OCR1A = DCC_ONE_BIT_TOTAL_DURATION_TIMER1;
    OCR1B = DCC_ONE_BIT_PULSE_DURATION_TIMER1;

    pinMode(DCCppConfig::SignalEnablePinMain, OUTPUT); // master enable for motor channel A

    mainRegs.loadPacket(1, RegisterList::idlePacket, 2, 0); // load idle packet into register 1

    bitSet(TIMSK1, OCIE1B); // enable interrupt vector for Timer 1 Output Compare B Match (OCR1B)
    digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);
}

void DCCpp::beginProg(uint8_t inOptionalDirectionMotor, uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
    DCCppConfig::SignalEnablePinProg = inSignalEnable;
    DCCppConfig::CurrentMonitorProg = inCurrentMonitor;

    // If no programming line, exit.
    if (DCCppConfig::SignalEnablePinProg == UNDEFINED_PIN)
    {
        return;
    }

    progMonitor.begin(DCCppConfig::CurrentMonitorProg, (char *)"<p4>");

    // Configuration for MEGA
    // Direction Pin for Motor Shield Channel B - PROGRAMMING TRACK
    // Controlled by Arduino 16-bit TIMER 3 / OC3B Interrupt Pin
    // Values for 16-bit OCR3A and OCR3B registers calibrated for 1:1 prescale at 16 MHz clock frequency
    // Resulting waveforms are 200 microseconds for a ZERO bit and 116 microseconds for a ONE bit with exactly 50% duty cycle

#define DCC_ZERO_BIT_TOTAL_DURATION_TIMER3 3199
#define DCC_ZERO_BIT_PULSE_DURATION_TIMER3 1599

#define DCC_ONE_BIT_TOTAL_DURATION_TIMER3 1855
#define DCC_ONE_BIT_PULSE_DURATION_TIMER3 927

    pinMode(DCC_SIGNAL_PIN_PROG, OUTPUT); // THIS ARDUINO OUTPUT PIN MUST BE PHYSICALLY CONNECTED TO THE PIN FOR DIRECTION-B OF MOTOR CHANNEL-B

    bitSet(TCCR3A, WGM30); // set Timer 3 to FAST PWM, with TOP=OCR3A
    bitSet(TCCR3A, WGM31);
    bitSet(TCCR3B, WGM32);
    bitSet(TCCR3B, WGM33);

    bitSet(TCCR3A, COM3B1); // set Timer 3, OC3B (pin 2) to inverting toggle (actual direction is arbitrary)
    bitSet(TCCR3A, COM3B0);

    bitClear(TCCR3B, CS32); // set Timer 3 prescale=1
    bitClear(TCCR3B, CS31);
    bitSet(TCCR3B, CS30);

    OCR3A = DCC_ONE_BIT_TOTAL_DURATION_TIMER3;
    OCR3B = DCC_ONE_BIT_PULSE_DURATION_TIMER3;

    pinMode(DCCppConfig::SignalEnablePinProg, OUTPUT); // master enable for motor channel B

    progRegs.loadPacket(1, RegisterList::idlePacket, 2, 0); // load idle packet into register 1

    bitSet(TIMSK3, OCIE3B); // enable interrupt vector for Timer 3 Output Compare B Match (OCR3B)

    digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);
}

void DCCpp::begin()
{
    programMode = false;
    panicStopped = false;

    DCCppConfig::SignalEnablePinMain = UNDEFINED_PIN;
    DCCppConfig::CurrentMonitorMain = UNDEFINED_PIN;

    DCCppConfig::SignalEnablePinProg = UNDEFINED_PIN;
    DCCppConfig::CurrentMonitorProg = UNDEFINED_PIN;

    mainMonitor.begin(UNDEFINED_PIN, "");
    progMonitor.begin(UNDEFINED_PIN, "");

} // begin

///////////////////////////////////////////////////////////////////////////////
// DEFINE THE INTERRUPT LOGIC THAT GENERATES THE DCC SIGNAL
///////////////////////////////////////////////////////////////////////////////

// The code below will be called every time an interrupt is triggered on OCNB, where N can be 0 or 1.
// It is designed to read the current bit of the current register packet and
// updates the OCNA and OCNB counters of Timer-N to values that will either produce
// a long (200 microsecond) pulse, or a short (116 microsecond) pulse, which respectively represent
// DCC ZERO and DCC ONE bits.

// These are hardware-driven interrupts that will be called automatically when triggered regardless of what
// DCC++ BASE STATION was otherwise processing.  But once inside the interrupt, all other interrupt routines are temporarily disabled.
// Since a short pulse only lasts for 116 microseconds, and there are TWO separate interrupts
// (one for Main Track Registers and one for the Program Track Registers), the interrupt code must complete
// in much less than 58 microseconds, otherwise there would be no time for the rest of the program to run.  Worse, if the logic
// of the interrupt code ever caused it to run longer than 58 microseconds, an interrupt trigger would be missed, the OCNA and OCNB
// registers would not be updated, and the net effect would be a DCC signal that keeps sending the same DCC bit repeatedly until the
// interrupt code completes and can be called again.

// A significant portion of this entire program is designed to do as much of the heavy processing of creating a properly-formed
// DCC bit stream upfront, so that the interrupt code below can be as simple and efficient as possible.

// Note that we need to create two very similar copies of the code --- one for the Main Track OC1B interrupt and one for the
// Programming Track OCOB interrupt.  But rather than create a generic function that incurs additional overhead, we create a macro
// that can be invoked with proper parameters for each interrupt.  This slightly increases the size of the code base by duplicating
// some of the logic for each interrupt, but saves additional time.

// As structured, the interrupt code below completes at an average of just under 6 microseconds with a worse-case of just under 11 microseconds
// when a new register is loaded and the logic needs to switch active register packet pointers.

// THE INTERRUPT CODE MACRO:  R=REGISTER LIST (mainRegs or progRegs), and N=TIMER (0 or 1)

#define DCC_SIGNAL(R, N)                                                                                                                                                            \
    if (R.currentBit == R.currentReg->activePacket->nBits)                                                                                                                          \
    {                     /* IF no more bits in this DCC Packet */                                                                                                                  \
        R.currentBit = 0; /*   reset current bit pointer and determine which Register and Packet to process next--- */                                                              \
        if (R.nRepeat > 0 && R.currentReg == R.reg)                                                                                                                                 \
        {                /*   IF current Register is first Register AND should be repeated */                                                                                       \
            R.nRepeat--; /*     decrement repeat count; result is this same Packet will be repeated */                                                                              \
        }                                                                                                                                                                           \
        else if (R.nextReg != NULL)                                                                                                                                                 \
        {                                              /*   ELSE IF another Register has been updated */                                                                            \
            R.currentReg = R.nextReg;                  /*     update currentReg to nextReg */                                                                                       \
            R.nextReg = NULL;                          /*     reset nextReg to NULL */                                                                                              \
            R.tempPacket = R.currentReg->activePacket; /*     flip active and update Packets */                                                                                     \
            R.currentReg->activePacket = R.currentReg->updatePacket;                                                                                                                \
            R.currentReg->updatePacket = R.tempPacket;                                                                                                                              \
        }                                                                                                                                                                           \
        else                                                                                                                                                                        \
        {                                       /*   ELSE simply move to next Register */                                                                                           \
            if (R.currentReg == R.maxLoadedReg) /*     BUT IF this is last Register loaded */                                                                                       \
                R.currentReg = R.reg;           /*       first reset currentReg to base Register, THEN */                                                                           \
            R.currentReg++;                     /*       increment current Register (note this logic causes Register[0] to be skipped when simply cycling through all Registers) */ \
        } /*   END-ELSE */                                                                                                                                                          \
    } /* END-IF: currentReg, activePacket, and currentBit should now be properly set to point to next DCC bit */                                                                    \
                                                                                                                                                                                    \
    if (R.currentReg->activePacket->buf[R.currentBit / 8] & R.bitMask[R.currentBit % 8])                                                                                            \
    {                                                    /* IF bit is a ONE */                                                                                                      \
        OCR##N##A = DCC_ONE_BIT_TOTAL_DURATION_TIMER##N; /*   set OCRA for timer N to full cycle duration of DCC ONE bit */                                                         \
        OCR##N##B = DCC_ONE_BIT_PULSE_DURATION_TIMER##N; /*   set OCRB for timer N to half cycle duration of DCC ONE but */                                                         \
    }                                                                                                                                                                               \
    else                                                                                                                                                                            \
    {                                                     /* ELSE it is a ZERO */                                                                                                   \
        OCR##N##A = DCC_ZERO_BIT_TOTAL_DURATION_TIMER##N; /*   set OCRA for timer N to full cycle duration of DCC ZERO bit */                                                       \
        OCR##N##B = DCC_ZERO_BIT_PULSE_DURATION_TIMER##N; /*   set OCRB for timer N to half cycle duration of DCC ZERO bit */                                                       \
    } /* END-ELSE */                                                                                                                                                                \
                                                                                                                                                                                    \
    R.currentBit++; /* point to next bit in current Packet */

///////////////////////////////////////////////////////////////////////////////
// NOW USE THE ABOVE MACRO TO CREATE THE CODE FOR EACH INTERRUPT

ISR(TIMER1_COMPB_vect){// set interrupt service for OCR1B of TIMER-1 which flips direction bit of Motor Shield Channel A controlling Main Track
                       DCC_SIGNAL(DCCpp::mainRegs, 1)}

// Configuration for MEGA
ISR(TIMER3_COMPB_vect)
{ // set interrupt service for OCR3B of TIMER-3 which flips direction bit of Motor Shield Channel B controlling Programming Track
    DCC_SIGNAL(DCCpp::progRegs, 3)
}

void DCCpp::panicStop(bool inStop)
{
    panicStopped = inStop;

    /* activate or not the power on rails */

    if (inStop)
        powerOff();
    else
        powerOn();
}

void DCCpp::powerOn()
{
    if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
        digitalWrite(DCCppConfig::SignalEnablePinProg, HIGH);

    if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
    {
        digitalWrite(DCCppConfig::SignalEnablePinMain, HIGH);
    }

    DCCPP_INTERFACE.println("<p1>");

    DCCpp::pingSend = false;
    DCCpp::pingTime = millis();
    DCCpp::panicStopped = false;
}

void DCCpp::powerOff()
{
    if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
        digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);
    if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
        digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);
    DCCPP_INTERFACE.println("<p0>");

    stopAllThrottles();
}

/***************************** Driving functions */

bool DCCpp::setThrottle(volatile RegisterList *inpRegs, int nReg, int inLocoId, int inStepsNumber, int inNewSpeed, bool inForward)
{
    int val = 0;

    if (panicStopped)
        val = 1;
    else if (inNewSpeed > 0)
        val = map(inNewSpeed, 0, inStepsNumber, 2, 127);

    inpRegs->setThrottle(nReg, inLocoId, val, inForward);

    return true;
}

void DCCpp::stopAllThrottles()
{
    for (int i = 0; i <= MAX_MAIN_REGISTERS; i++)
    {
        if (DCCpp::mainRegs.speedTable[i] == 0)
            continue;

        DCCpp::mainRegs.setThrottle(i, DCCpp::mainRegs.addrTable[i], 0, 0);
    }
}

void DCCpp::setFunctions(volatile RegisterList *inpRegs, int nReg, int inLocoId, FunctionsState &inStates)
{
    byte flags = 0;

    byte oneByte1 = 128;   // Group one functions F0-F4
    byte twoByte1 = 176;   // Group two F5-F8
    byte threeByte1 = 160; // Group three F9-F12
    byte fourByte2 = 0;    // Group four F13-F20
    byte fiveByte2 = 0;    // Group five F21-F28

    for (byte func = 0; func <= 28; func++)
    {
        if (func <= 4)
        {
            /*
             *   To set functions F0 - F4 on(= 1) or off(= 0) :
             *
             *    BYTE1 : 128 + F1 * 1 + F2 * 2 + F3 * 4 + F4 * 8 + F0 * 16
             * BYTE2 : omitted
             */

            if (inStates.isActivationChanged(func))
                flags |= 1;
            if (inStates.isActivated(func))
            {
                if (func == 0)
                    oneByte1 += 16;
                else
                    oneByte1 += (1 << (func - 1));
            }
        }
        else if (func <= 8)
        {
            /*
             *   To set functions F5 - F8 on(= 1) or off(= 0) :
             *
             *    BYTE1 : 176 + F5 * 1 + F6 * 2 + F7 * 4 + F8 * 8
             * BYTE2 : omitted
             */

            if (inStates.isActivationChanged(func))
                flags |= 2;
            if (inStates.isActivated(func))
                twoByte1 += (1 << (func - 5));
        }
        else if (func <= 12)
        {
            /*
             *    To set functions F9 - F12 on(= 1) or off(= 0) :
             *
             *    BYTE1 : 160 + F9 * 1 + F10 * 2 + F11 * 4 + F12 * 8
             * BYTE2 : omitted
             */

            if (inStates.isActivationChanged(func))
                flags |= 4;
            if (inStates.isActivated(func))
                threeByte1 += (1 << (func - 9));
        }
        else if (func <= 20)
        {
            /*
             *    To set functions F13 - F20 on(= 1) or off(= 0) :
             *
             *    BYTE1 : 222
             * BYTE2 : F13 * 1 + F14 * 2 + F15 * 4 + F16 * 8 + F17 * 16 + F18 * 32 + F19 * 64 + F20 * 128
             */

            if (inStates.isActivationChanged(func))
                flags |= 8;
            if (inStates.isActivated(func))
                fourByte2 += (1 << (func - 13));
        }
        else if (func <= 28)
        {
            /*
             *    To set functions F21 - F28 on(= 1) of off(= 0) :
             *
             *    BYTE1 : 223
             * BYTE2 : F21 * 1 + F22 * 2 + F23 * 4 + F24 * 8 + F25 * 16 + F26 * 32 + F27 * 64 + F28 * 128
             */

            if (inStates.isActivationChanged(func))
                flags |= 16;
            if (inStates.isActivated(func))
                fiveByte2 += (1 << (func - 21));
        }
    }

    if (flags & 1)
        inpRegs->setFunction(nReg, inLocoId, oneByte1, -1);
    if (flags & 2)
        inpRegs->setFunction(nReg, inLocoId, twoByte1, -1);
    if (flags & 4)
        inpRegs->setFunction(nReg, inLocoId, threeByte1, -1);
    if (flags & 8)
        inpRegs->setFunction(nReg, inLocoId, 222, fourByte2);
    if (flags & 16)
        inpRegs->setFunction(nReg, inLocoId, 223, fiveByte2);

    inStates.statesSent();
}

int DCCpp::identifyLocoId(volatile RegisterList *inReg)
{
    int id = -1;
    int temp;
    temp = inReg->readCV(29, 100, 200);
    if ((temp != -1) && (bitRead(temp, 5)))
    {
        // long address : get CV#17 and CV#18
        id = inReg->readCV(18, 100, 200);
        if (id != -1)
        {
            temp = inReg->readCV(17, 100, 200);
            if (temp != -1)
            {
                id = id + ((temp - 192) << 8);
            }
        }
    }
    else
    {
        // short address: read only CV#1
        id = inReg->readCV(1, 100, 200);
    }
    return (id);
}

void DCCpp::writeCv(volatile RegisterList *inReg, int inCv, byte inValue, int callBack, int callBackSub)
{
    inReg->writeCVByte(inCv, inValue, callBack, callBackSub);
}

void DCCpp::setAccessory(int inAddress, byte inSubAddress, byte inActivate)
{
    mainRegs.setAccessory(inAddress, inSubAddress, inActivate);
}

void DCCpp::setTurnout(char *c)
{
    // <T ADDRESS STATE> : throw the turnout at the given linear address.
    //
    // ADDRESS is a linear turnout number (1..2044), mapped onto the NMRA
    // accessory address / sub-address pair the same way JMRI and DCC++ EX do
    // it. This station does not store turnout definitions, so the three
    // argument form <T ID ADDRESS SUBADDRESS> that some software sends to
    // declare a turnout has no meaning here and is rejected rather than
    // silently throwing a turnout: with the previous parser it was read as
    // <T ID STATE> with STATE = ADDRESS, which fired the turnout.
    int address, state, extra;

    // Exactly two arguments are expected. Reading a third one lets us tell a
    // definition apart from a throw order, and strtol() could not: it returns
    // 0 for an empty string just as it does for "0", so a bare <T> used to
    // emit a real accessory packet on address 0.
    if (sscanf(c, "%d %d %d", &address, &state, &extra) != 2)
    {
        DCCPP_INTERFACE.println("<X>");
        return;
    }

    if (address < 1 || address > 2044)
    {
        DCCPP_INTERFACE.println("<X>");
        return;
    }

    int linear = address + 3;

    DCCpp::mainRegs.setAccessory((linear >> 2), (linear & 3), (state > 0));
    DCCPP_INTERFACE.println("<H " + String(address) + ((state > 0) ? " 1>" : " 0>"));
}