/**********************************************************************

CurrentMonitor.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef CurrentMonitor_h
#define CurrentMonitor_h

/** Factor to smooth the result. Tied to CURRENT_SAMPLE_TIME below: the filter
settles in roughly CURRENT_SAMPLE_TIME / CURRENT_SAMPLE_SMOOTHING milliseconds,
so the two must be changed together to keep the same reaction time. */
#define CURRENT_SAMPLE_SMOOTHING 0.1

/** Time between two measurements, in milliseconds. analogRead() blocks for
about 112 us on this part, so a one millisecond period spent most of the CPU
inside the ADC. 10 ms with the smoothing above keeps the same ~100 ms reaction
time to an overload for a tenth of the cost. */
#define CURRENT_SAMPLE_TIME 10

/** This structure/class describes a current monitor.*/

struct CurrentMonitor
{
  static long int sampleTime; /**< time elapsed since last measurement. This delay is common to all monitors. */
  int pin;                    /**< Attached pin.*/
  float currentSampleMax;     /**< Value of the maximum current accepted without problem.*/
  float current;              /**< Value of the last measured current.*/
  const char *msg;            /**< Message sent when excessive current is measured: <p2> for main, <p4> for prog.*/
  /** begin function.
  @param pin    Attached pin. UNDEFINED_PIN to inactivate this monitor.
  @param msg    Message to send to console when a smoothed current value greater than maximum is detected.
  @param inSampleMax    Maximum value of the current. Default is 300.
  */
  void begin(int pin, const char *msg, float inSampleMax = 300);
  /** Checks if sufficient time has elapsed since last update. Common to all monitors.
   */
  static boolean checkTime();
  /** Reads the emergency stop inputs of the station. These belong to the
  station as a whole rather than to one district, so this is called once per
  loop, not once per monitor.
   */
  static void checkSafetyInputs();
  /** Checks the current on this monitor.
   */
  void check();
};

#endif
