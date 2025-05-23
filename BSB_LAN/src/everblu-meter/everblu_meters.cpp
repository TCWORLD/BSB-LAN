
#include "src/everblu-meter/everblu_meters.h" // Include EverBlu meter communication library

#if defined(ESP32)
#include <esp_task_wdt.h>
#endif
  
// Function to scan for the correct frequency in the 433 MHz range
//  - returns frequency or negative if not found.
static float scanFrequency433MHz() {
    printToDebug("###### FREQUENCY DISCOVERY ENABLED (433 MHz) ######\nStarting Frequency Scan...\n");
    float fStart = -1.0f;
    float fEnd = -1.0f;
    for (float fCheck = 433.76f; fCheck < 433.890f; fCheck += 0.0005f) {
#if defined(ESP32)
        esp_task_wdt_reset();
#endif
        printFmtToDebug("Test frequency : %f\n", fCheck);
        cc1101_init(fCheck);
        struct tmeter_data meter_data = get_meter_data();
        if (meter_data.error > 0) {
            // Found a meter
            if (fStart < 0) {
                // If this is the first one, log the start of valid frequency range
                fStart = fCheck;
                fEnd = fCheck;
                printFmtToDebug("\n------------------------------\nFirst valid frequency : %f\n------------------------------\n", fCheck);
                printFmtToDebug("Liters : %d\nBattery (in months) : %d\nCounter : %d\n\n", meter_data.liters, meter_data.battery_left, meter_data.reads_counter);
            } else {
                // Otherwise keep updating the end point
                fEnd = fCheck;
            }
        } else if (fStart > 0) {
            // If we were in a valid frequency band, then an error now indicates we have gone beyond the valid range.
            printFmtToDebug("\n------------------------------\nLast valid frequency : %f\n------------------------------\n", fEnd);
            break;
        }
    }
    if (fStart > 0) {
        // If we found a valid frequency band, then pick the centre of the band.
        float fMid = (fEnd + fStart) / 2.0f;
        printFmtToDebug("\n------------------------------\nSelected frequency : %f\n------------------------------\n", fMid);
        return fMid;
    } else {
        printToDebug("\n------------------------------\nFailed to find meter in scan range.\n------------------------------\n");
        return -1.0f;
    }
}

void printMeterData(struct tmeter_data *data) {
    printFmtToDebug("Consumption   : %d Liters\n", data->liters);
    printFmtToDebug("Battery left  : %d Months\n", data->battery_left);
    printFmtToDebug("Read counter  : %d times\n", data->reads_counter);
    printFmtToDebug("Working hours : from %02dH to %02d\n", data->time_start, data->time_end);
    printFmtToDebug("Frequency     : %0.4f\n", data->frequency);
    printFmtToDebug("RSSI  /  LQI  : %ddBm  /  %d\n", data->rssi, data->lqi);
}

static everblu_config_t *config = NULL;

void everblu_setConfig(everblu_config_t *cfg) {
    config = cfg;
}

float everblu_getFrequency() {
    if (!config)
        return -1.0f;
    return config->frequency;
}

bool everblu_initialise(long *meter_data) {
    // Fail if config not set
    if (!config)
        return false;
    // Check if initialised, and if not initialise it.
    if (config->cfgInit != 0xEB) {
        memset(config, 0, sizeof(*config));
        config->frequency = -1.0f;
        config->cfgInit = 0xEB;
    }
    // Populate meter data with last known values if requested
    if (meter_data) {
        meter_data[EVERBLU_METER_LITRES] = config->lastLitres;
        meter_data[EVERBLU_METER_READCOUNT] = config->lastReadCount;
        meter_data[EVERBLU_METER_BATTERY] = config->lastBatteryLeft;
    }
    // Perform frequency scan if not known, or out of range, rescan
    if ((config->frequency < 433.0f) || (config->frequency > 434.0f)) {
        config->frequency = scanFrequency433MHz();
        // If still unknown, fail initialise.
        if (config->frequency < 0.0f)
            return false;
    }
    // Initialise the controller to the selected frequency
    return cc1101_init(config->frequency);
}

bool everblu_readMeter(long *meter_data) {
#if defined(ESP32)
    esp_task_wdt_reset();
#endif
    // Try to read the meter
    struct tmeter_data meter_read = get_meter_data();
    if (meter_read.error <= 0)
        return false;
    
    // Print the raw data
    printMeterData(&meter_read);
    
    // Update last known values
    config->lastLitres = meter_read.liters;
    config->lastReadCount = meter_read.battery_left;
    config->lastBatteryLeft = meter_read.reads_counter;
        
    // And return data if required
    if (meter_data) {
        meter_data[EVERBLU_METER_LITRES] = config->lastLitres;
        meter_data[EVERBLU_METER_READCOUNT] = config->lastReadCount;
        meter_data[EVERBLU_METER_BATTERY] = config->lastBatteryLeft;
    }
    return true;
}
