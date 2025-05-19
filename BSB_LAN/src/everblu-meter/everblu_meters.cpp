
#include "src/everblu-meter/everblu_meters.h" // Include EverBlu meter communication library

static everblu_config_t *config = NULL;

// Extract meter data from config
static void everblu_lastKnownMeterRead(long* meter_data) {
    if (meter_data) {
        meter_data[EVERBLU_METER_LITRES] = config->lastLitres;
        meter_data[EVERBLU_METER_READCOUNT] = config->lastReadCount;
        meter_data[EVERBLU_METER_BATTERY] = config->lastBatteryLeft;
    }
}

// Extracts meter structure and populates config and meter data export
static void everblu_populateMeterData(struct tmeter_data* meter_read, long *meter_data) {
    
    // Print the raw data
    printFmtToDebug("Consumption   : %d Liters\n", meter_read->liters);
    printFmtToDebug("Battery left  : %d Months\n", meter_read->battery_left);
    printFmtToDebug("Read counter  : %d times\n", meter_read->reads_counter);
    printFmtToDebug("Working hours : from %02dH to %02d\n", meter_read->time_start, meter_read->time_end);
    printFmtToDebug("Frequency     : %0.4f\n", meter_read->frequency);
    printFmtToDebug("RSSI/CRC/LQI  : %ddBm / %s / %d\n", meter_read->rssi, meter_read->crcok ? "OK" : "FAIL", meter_read->lqi);
    
    // Update last known values
    config->lastLitres = meter_read->liters;
    config->lastReadCount = meter_read->reads_counter;
    config->lastBatteryLeft = meter_read->battery_left;
        
    // And return data if required
    everblu_lastKnownMeterRead(meter_data);
}

// Function to scan for the correct frequency in the 433 MHz range
//  - Non-blocking. Will perform one frequency check per call
//  - set restart to true to restart scanning from first frequency. 
//       - Will not perform scan, just resets counters.
//  - returns -1.0f if all frequencies scanned but nothing found
//  - returns -fCheck, indicating next frequency to be scanned
//  - returns +ve frequency if valid frequency is found.
//  - If provided, meter_data will be populated with any data found
//    during a successful scan.
float everblu_scanFrequency433MHz(bool restart, long *meter_data) {
    static float fStart = -1.0f;
    static float fEnd = -1.0f;
    static float fCheck = SWEEP_FREQUENCY_MIN;
    static bool scanDone = false;
    if (restart) {
        // Restart scan counters
        printToDebug("###### FREQUENCY DISCOVERY ENABLED (433 MHz) ######\nReady to run new sweep.\n");
        fStart = -1.0f;
        fEnd = -1.0f;
        fCheck = SWEEP_FREQUENCY_MIN;
        scanDone = false;
        return -fCheck;
    } else if (scanDone) {
        // Skip if no scan running
        printToDebug("\n------------------------------\nNo frequency sweep running. Skipping.\n------------------------------\n");
        return -1.0f;
    }
    printFmtToDebug("\n------------------------------\nStarting next frequency sweep step at: %f.\n------------------------------\n", fCheck);
    
    // Check if still within check range
    if (fCheck < SWEEP_FREQUENCY_MAX) {
        printFmtToDebug("Test frequency : %f\n", fCheck);
        cc1101_init(fCheck);
        struct tmeter_data meter_read = get_meter_data();
        if (meter_read.error > 0) {
            // Found a meter
            if (fStart < 0) {
                // If this is the first one, log the start of valid frequency range
                fStart = fCheck;
                fEnd = fCheck;
                printFmtToDebug("\n------------------------------\nFirst valid frequency : %f\n------------------------------\n", fCheck);
                everblu_populateMeterData(&meter_read, meter_data);
            } else {
                // Otherwise keep updating the end point
                fEnd = fCheck;
            }
            fCheck += SWEEP_FREQUENCY_STEP;
            return -fCheck;
        } else if (fStart > 0) {
            // If we were in a valid frequency band, then an error now indicates we have gone beyond the valid range.
            printFmtToDebug("\n------------------------------\nLast valid frequency : %f\n------------------------------\n", fEnd);
            fCheck = SWEEP_FREQUENCY_MAX;
        } else {
            // Not valid. Move to next frequency and return that scan still in progress.
            fCheck += SWEEP_FREQUENCY_STEP;
            return -fCheck;
        }   
    }
    
    // Scan just finished. Mark as done
    scanDone = true;
    cc1101_sleep(); // Finished with the RF for now.
    if (fStart > 0) {
        // If we found a valid frequency band, then pick the centre of the band.
        float fMid = (fEnd + fStart) / 2.0f;
        printFmtToDebug("\n------------------------------\nSelected frequency : %f\n------------------------------\n", fMid);
        return fMid;
    } else {
        // Otherwise log failure
        printToDebug("\n------------------------------\nFailed to find meter in scan sweep range.\nAre you in range and in business hours?\n------------------------------\n");
        return -1.0f;
    }
}

void everblu_setConfig(everblu_config_t *cfg) {
    config = cfg;
}

float everblu_getFrequency() {
    if (!config)
        return -1.0f;
    return config->frequency;
}

// Set selected frequency for meter
//  - Returns false if invalid.
//  - Set a frequency of -1.0f to retrigger autodetection.
bool everblu_setFrequency(float frequency) {
    // Error if config not yet set
    if (!config) {
        return false;
    }
    // Error if not valid frequency
    if ((frequency != -1.0f) && ((frequency < CC1101_FREQUENCY_MIN) || (frequency > CC1101_FREQUENCY_MAX))) {
        return false;
    }
    // Set the new frequency
    config->frequency = frequency;
    return true;
}

bool everblu_initialise(long *meter_data) {
    // Fail if config not set
    if (!config)
        return false;
    // Check if initialised, and if not initialise it.
    if (config->cfgInit != 0xEB || !config->cfgVer || !config->cfgSize) {
        memset(config, 0, sizeof(*config));
        config->enable = 1; // Default to enabled
        config->frequency = -1.0f; // With no known frequency
        config->cfgSize = sizeof(*config);
        config->cfgVer = EVERBLU_CONFIG_VERSION;
        config->cfgInit = 0xEB; // Set magic word to mark as initialised
    }
    // If config has grown, zero out new space
    if (config->cfgSize < sizeof(*config)) {
        memset(((uint8_t*)config) + config->cfgSize, 0, sizeof(*config) - config->cfgSize);
    }
    // If size has changed update size and version fields.
    if ((config->cfgSize != sizeof(*config)) || (config->cfgVer != EVERBLU_CONFIG_VERSION)) {
        config->cfgVer = EVERBLU_CONFIG_VERSION;
        config->cfgSize = sizeof(*config);
    }
    // Populate meter data with last known values if requested
    everblu_lastKnownMeterRead(meter_data);
    // Reset frequency to unknwon if out of range
    if ((config->frequency < CC1101_FREQUENCY_MIN) || (config->frequency > CC1101_FREQUENCY_MAX)) {
        config->frequency = -1.0f;
    }
    // Check that hardware exists without actually configuring the RF
    bool hasRf = cc1101_init(-1.0f);
    // Restart scanner counters without performing scan
    everblu_scanFrequency433MHz(true);
    cc1101_sleep();
    return hasRf && config->enable;
}

// Read the meter.
//  - If LastKnown is true, then will return values from config and not try and talk to meter
//  - Otherwise will talk to meter, and if successful, last known values in config will be updated.
//  - if non-null, will populate meter_data array. Must be EVERBLU_DATA_COUNT elements long.
bool everblu_readMeter(bool lastKnown, long* meter_data) {
    if (lastKnown) {
        everblu_lastKnownMeterRead(meter_data);
        return true;
    }    
    
    // Initialise the controller to the selected frequency if known
    float frequency = everblu_getFrequency();
    if (frequency < 0.0f) return false; // Can't run, don't know freq.
    if (!cc1101_init(frequency)) return false; // Initialise to desired frequency and return if failed.

    // Try to read the meter
    struct tmeter_data meter_read = get_meter_data();
    if (meter_read.error <= 0)
        return false;
    
    // Populate meter data array and save to config.
    everblu_populateMeterData(&meter_read, meter_data);
    
    // Put to sleep
    cc1101_sleep();
    return true;
}
