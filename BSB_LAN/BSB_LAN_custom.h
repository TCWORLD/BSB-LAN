/* 
 * Attempt to read the value from an everblu water meter once per day.
*/

// If we have the RF interface, and the frequency line is valid
if (has_everblu && everblu_setFrequency(custom_floats[0])) {
  
  // Check if we have a frequency set
  if (custom_floats[0] == -1.0f) {
    // Meter scanning mode
    
    // Perform next scan
    float scanResult = everblu_scanFrequency433MHz(false);
    if (scanResult >= 0.0f) {
      // Non-negative means a frequency was found.
      custom_floats[0] = scanResult;
      custom_floats[1] = 0.0f; // Scan complete
    } else if (scanResult == -1.0f) {
      // Scan failed to find meter.
      custom_floats[0] = 0.0f; // Set invalid frequency in register. This must be changed to -1.0f again by user to restart scan
      custom_floats[1] = -1.0f; // Scan failed
      everblu_scanFrequency433MHz(true); // Reset scan paramters
    } else {
      // Scan still in progress. Report next check frequency to web interface for debugging
      custom_floats[1] = -scanResult;
    }
  
  } else {
    // Meter reading mode
  
    // Try to read the meter once per hour until a successful read
    // We try for at most 6 hours.
    static uint8_t meterReadRetries = 6;
    if (meterReadRetries) {
      // If there are retries remaining, keep trying to read once per hour
      if ((custom_timer - custom_timer_compare) >= 3600000) {    // every 3600 seconds
        custom_timer_compare = custom_timer;
        printToDebug("Attempting to read water meter!\r\n");
        // Try reading
        if (everblu_readMeter(custom_longs)) {
          // Success. No more retries until next day
          meterReadRetries = 0;
        } else {
          // Failed read. One less try remaining.
          meterReadRetries--;
        }
      }
    }

    // If there are no retries remaining, check if we need to restart reading
    if (!meterReadRetries) {
      // Check the current hour
      int currentHour;
      #if defined(ESP32)
      struct tm now;
      getLocalTime(&now,100);
      currentHour = now.tm_hour;
      #else
      currentHour = hour();
      #endif
      // If it equals the restart hour
      if (currentHour == METER_READ_HOUR) {
        // Then allow reading the meter again.
        meterReadRetries = 6;
      }
    }
  
  }
}
