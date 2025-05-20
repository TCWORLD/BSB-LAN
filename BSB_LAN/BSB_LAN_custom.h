/* 
 * Attempt to read the value from an everblu water meter once per day.
*/

// If we have the RF interface, and the frequency line is valid
if (has_everblu && everblu_setFrequency(custom_floats[METER_CUSTOMFLOAT_METER_FREQUENCY])) {
  
  static uint8_t meterReadRetries = 0;
  
  // Check the current hour
  int theHour, theDayOfWeek;
#if defined(ESP32)
  struct tm now;
  getLocalTime(&now,100);
  theHour = now.tm_hour;
  theDayOfWeek = now.tm_wday;
#else
  theHour = hour();
  theDayOfWeek = weekday() - 1;
#endif
  custom_longs[METER_CUSTOMLONG_CURRENT_HOUR] = theHour;
  custom_longs[METER_CUSTOMLONG_CURRENT_DAY] = theDayOfWeek;
  
  // Check if we have a frequency set
  if (custom_floats[METER_CUSTOMFLOAT_METER_FREQUENCY] == -1.0f) {
    // Meter scanning mode
    
    // Perform next scan
    float scanResult = everblu_scanFrequency433MHz(false, &custom_longs[METER_CUSTOMLONG_METER_DATA]);
    if (scanResult >= 0.0f) {
      // Non-negative means a frequency was found.
      custom_floats[METER_CUSTOMFLOAT_METER_FREQUENCY] = scanResult;
      custom_floats[METER_CUSTOMFLOAT_SCAN_FREQUENCY] = 0.0f; // Scan complete
      meterReadRetries = 0; // No retries until next read day as we've got the meter reading.
      writeToEEPROM(CF_CUSTOM_EEPROM); // Commit new frequency config to EEPROM
    } else if (scanResult == -1.0f) {
      // Scan failed to find meter.
      custom_floats[METER_CUSTOMFLOAT_METER_FREQUENCY] = 0.0f; // Set invalid frequency in register. This must be changed to -1.0f again by user to restart scan
      custom_floats[METER_CUSTOMFLOAT_SCAN_FREQUENCY] = -1.0f; // Scan failed
      everblu_scanFrequency433MHz(true); // Reset scan paramters
    } else {
      // Scan still in progress. Report next check frequency to web interface for debugging
      custom_floats[METER_CUSTOMFLOAT_SCAN_FREQUENCY] = -scanResult;
    }
  
  } else {
    // Meter reading mode
  
    // Try to read the meter once per hour until a successful read
    // We try for at most 6 hours.
    if (meterReadRetries) {
      // If there are retries remaining, keep trying to read once per hour
      if ((custom_timer - custom_timer_compare) >= 3600000) {    // every 3600 seconds
        custom_timer_compare = custom_timer;
        printToDebug("Attempting to read water meter!\r\n");
        // Try reading
        if (everblu_readMeter(false, &custom_longs[METER_CUSTOMLONG_METER_DATA])) {
          // Success. No more retries until next read day
          meterReadRetries = 0;
          writeToEEPROM(CF_CUSTOM_EEPROM); // Commit last known reading to EEPROM
        } else {
          // Failed read. One less try remaining.
          meterReadRetries--;
        }
      }
    }

    // If there are no retries remaining, check if we need to restart reading
    if (!meterReadRetries) {
      // If it equals the restart hour
      if (theHour == METER_READ_HOUR && ((METER_READ_WKDAY < 0) || (theDayOfWeek == METER_READ_WKDAY))) {
        // Then allow reading the meter again.
        meterReadRetries = 6;
      }
    }
  }
  
  custom_longs[METER_CUSTOMLONG_READ_ATTEMPTS] = meterReadRetries;
}
