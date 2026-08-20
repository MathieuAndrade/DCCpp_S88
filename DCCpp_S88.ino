#include "Arduino.h"
#include "DCCpp.h"
#include "Config.h"

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  Serial.println(F("DCCpp station starting..."));

  // Power district setup
  pinMode(EmergencyStop, INPUT_PULLUP);
#ifdef USE_CDE_BOOSTER
  pinMode(E_BoosterIn, INPUT);
#endif

  // S88 setup
  pinMode(S88_LOAD_PS_PIN, OUTPUT);
  pinMode(S88_Reset_PIN, OUTPUT);
  pinMode(S88_Clock_PIN, OUTPUT);
  pinMode(S88_DataL_PIN, INPUT);
  pinMode(S88_DataR_PIN, INPUT);

  digitalWrite(S88_LOAD_PS_PIN, LOW);
  digitalWrite(S88_Clock_PIN, LOW);
  digitalWrite(S88_Reset_PIN, LOW);

  DCCpp::begin();

  DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 14, A0);
  DCCpp::beginProg(UNDEFINED_PIN, DCC_SIGNAL_PIN_PROG, 11, A1);

  Serial.println(F("<DCCpp station ready>"));
}

void loop()
{
  DCCpp::loop();
}
