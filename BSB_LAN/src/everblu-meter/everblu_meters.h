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

typedef struct {
    uint8_t cfgInit; // Magic number to ensure config is initialised.
    float frequency; // Frequency of meter found during scanning
    int lastLitres;
    int lastReadCount;
    int lastBatteryLeft;
} everblu_config_t;

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
bool everblu_initialise(long* meter_data);

// Check selected frequency for meter
float everblu_getFrequency();

// Read the meter.
//  - Last known values in config will be updated.
//  - if non-null, will populate meter_data array. Must be EVERBLU_DATA_COUNT elements long.
bool everblu_readMeter(long* meter_data);

#endif
