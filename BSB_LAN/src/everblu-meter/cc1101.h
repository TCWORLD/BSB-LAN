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
bool cc1101_init(float freq, uint32_t freqx = 0, bool show = false);
void cc1101_sleep() ;
struct tmeter_data get_meter_data(void);

extern int _spi_speed;

#endif // __CC1101_H__