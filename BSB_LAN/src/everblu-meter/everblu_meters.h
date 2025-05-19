#ifndef _EVERBLU_METERS_H_
#define _EVERBLU_METERS_H_

#include <Arduino.h>

#include "everblu_config.h"
#include "everblu_utils.h"

#include "time.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#include "cc1101.h"

#define uS_TO_S_FACTOR 1000000ULL 

typedef enum {
    EVERBLU_METER_LITRES = 0,
    EVERBLU_METER_READCOUNT,
    EVERBLU_METER_BATTERY,
    // Number of words in meter data
    EVERBLU_DATA_COUNT
} everblu_data_t;

#define EVERBLU_CONFIG_VERSION 0x01

typedef struct __attribute__((packed)) {
    uint8_t cfgInit; // Magic number to ensure config is initialised.
    uint8_t cfgSize; // sizeof(everblu_config_t). Will zero bytes from cfgSize to end if different (e.g. config was expanded)
    uint8_t cfgVer;  // Config version
    uint8_t enable;
    float frequency; // Frequency of meter found during scanning
    int lastLitres;
    int lastReadCount;
    int lastBatteryLeft;
} everblu_config_t;

// Function to scan for the correct frequency in the 433 MHz range
//  - Non-blocking. Will perform one frequency check per call
//  - set restart to true to restart scanning from first frequency. 
//       - Will not perform scan, just resets counters.
//  - returns -1.0f if all frequencies scanned but nothing found
//  - returns -fCheck, indicating next frequency to be scanned
//  - returns +ve frequency if valid frequency is found.
//  - If provided, meter_data will be populated with any data found
//    during a successful scan.
float everblu_scanFrequency433MHz(bool restart, long *meter_data = NULL);

// Pass in a pointer to a RAM buffer which will be used to store config data
//  - cfg must point to memory which is at least sizeof(everblu_config_t) bytes long
void everblu_setConfig(everblu_config_t* cfg);

// Initialise the control code
//  - If config is not set, will fail initialise.
//  - If config memory is blank, will be initialised to defaults.
//  - Enables the CC1101 controller and perform frequency scan if necessary
//  - returns false if initialisation fails.
//  - if non-null, will populate meter_data with last known values. Array
//    must be EVERBLU_DATA_COUNT elements long.
bool everblu_initialise(long* meter_data = NULL);

// Set selected frequency for meter
//  - Returns false if invalid.
//  - Set a frequency of -1.0f to retrigger autodetection.
bool everblu_setFrequency(float frequency);

// Check selected frequency for meter
float everblu_getFrequency();

// Read the meter.
//  - If LastKnown is true, then will return values from config and not try and talk to meter
//  - Otherwise will talk to meter, and if successful, last known values in config will be updated.
//  - if non-null, will populate meter_data array. Must be EVERBLU_DATA_COUNT elements long.
bool everblu_readMeter(bool lastKnown, long* meter_data = NULL);

#endif
