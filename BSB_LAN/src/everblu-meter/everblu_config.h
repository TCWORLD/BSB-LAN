
#ifndef EVERBLU_CONFIG_H_
#define EVERBLU_CONFIG_H_

// Meter-specific configuration
#define METER_YEAR 13        // Last two digits of the year printed on the meter (e.g., 2019 is 19)
#define METER_SERIAL 535395  // Meter Serial Number (omit leading zero)
#define FREQUENCY 433.76f    // Frequency of the meter (default. Can be discovered via test code)
#define GDO0 33              // Pin used for GDO0 (General Digital Output 0)
#define GDO2 35              // Pin used for GDO2 (General Digital Output 2)

#define CC1101_FREQUENCY_MIN 433.0f // Minimum allowed frequency for CC1101 variant
#define CC1101_FREQUENCY_MAX 434.0f // Maximum allowed frequency for CC1101 variant

#define SWEEP_FREQUENCY_MIN 433.70f // Sweep frequency range lower limit
#define SWEEP_FREQUENCY_STEP 0.001f // Sweep frequency step
#define SWEEP_FREQUENCY_MAX 433.98f // Sweep frequency range upper limit


#define METER_READ_HOUR  9   // Hour to start trying to read meter
#define METER_READ_WKDAY 3   // Day of week to read meter. Sunday = 0. Setting to -1 attempts read every day.

#endif
