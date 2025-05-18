#ifndef __CC1101_H__
#define __CC1101_H__

#include <Arduino.h>        // Include the Arduino library for basic functions

struct tmeter_data {
  int liters;
  int reads_counter; // how many times the meter has been read
  int battery_left; // in months
  int time_start; // like 8 (8am, but in 24 hour format)
  int time_end; // like 18 (6pm, but in 24 hour format)
  int rssi; // Radio signal strength indicator
  int rssi_dbm; // RSSI in dBm
  int lqi; // Link quality indicator 0-255
	int error; // 0:No data 1:OK  -1:wrong data
  float frequency;
};

#define REG_DEFAULT 	0x10AF75 // CC1101 register values for 433.82MHz

void setMHZ(float mhz);
void setFREQxRegister(uint32_t freqx) ;


// Initialise CC1101
//  - If freq > 0, sets frequency to specified value in MHz
//  - If freq == 0, sets frequency using direct freqx register value
//  - If freq < 0, skips configuring RF. Allows for simply hardware presence check
bool cc1101_init(float freq, uint32_t freqx = 0, bool show = false);

// Configure CC1100 RF interface
//  - Use this if configuring was skipped during cc1101_init
//  - If freq == 0, sets frequency using direct freqx register value
//  - Else sets frequency to specified value in MHz
void cc1101_configureRF_0(float freq, uint32_t freqx);

// De-initialise CC1101 SPI
void cc1101_sleep() ;

// Scan meter data
struct tmeter_data get_meter_data(void);

extern int _spi_speed;

#endif // __CC1101_H__