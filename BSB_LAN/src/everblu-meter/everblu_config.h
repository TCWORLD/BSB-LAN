
#ifndef EVERBLU_CONFIG_H_
#define EVERBLU_CONFIG_H_

// Meter-specific configuration
#define METER_YEAR 13        // Last two digits of the year printed on the meter (e.g., 2019 is 19)
#define METER_SERIAL 535395  // Meter Serial Number (omit leading zero)
#define FREQUENCY 433.700007 // Frequency of the meter (default. Can be discovered via test code)
#define GDO0 35              // Pin used for GDO0 (General Digital Output 0)

#define METER_READ_HOUR  9   // Hour to start trying to read meter

#endif
