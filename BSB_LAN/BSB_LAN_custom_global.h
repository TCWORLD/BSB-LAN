// Add custom code for global functions here which will be included in the global section

#include "src/everblu-meter/everblu_meters.h" // Include EverBlu meter communication library

enum {
    METER_CUSTOMLONG_METER_DATA = 0,
    METER_CUSTOMLONG_READ_WKDAY = METER_CUSTOMLONG_METER_DATA + EVERBLU_DATA_COUNT,
    METER_CUSTOMLONG_READ_HOUR,
    METER_CUSTOMLONG_CURRENT_DAY,
    METER_CUSTOMLONG_CURRENT_HOUR,
    METER_CUSTOMLONG_READ_ATTEMPTS
};

enum {
    METER_CUSTOMFLOAT_METER_FREQUENCY = 0,
    METER_CUSTOMFLOAT_SCAN_FREQUENCY
};

bool has_everblu = false;
