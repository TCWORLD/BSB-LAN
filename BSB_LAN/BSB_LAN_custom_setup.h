// Add custom code for setup function here which will be included at the end of the function

// Set TX Enable pin to output and set high now that the system is initialised and the
// TX line is being driven correctly
pinMode(32, OUTPUT);
digitalWrite(32, HIGH);

// Initialise water meter reader
static_assert(sizeof(everblu_config_t) <= sizeof(custom_eeprom));
everblu_setConfig((everblu_config_t*)&custom_eeprom);
has_everblu = everblu_initialise(&custom_longs[METER_CUSTOMLONG_METER_DATA]);
// Save the frequency to MQTT line
if (has_everblu) {
    custom_floats[METER_CUSTOMFLOAT_METER_FREQUENCY] = everblu_getFrequency();
} else {
    custom_floats[METER_CUSTOMFLOAT_METER_FREQUENCY] = -1.0f;
}
custom_floats[METER_CUSTOMFLOAT_SCAN_FREQUENCY] = 0.0f;
custom_longs[METER_CUSTOMLONG_READ_WKDAY] = METER_READ_WKDAY;
custom_longs[METER_CUSTOMLONG_READ_HOUR] = METER_READ_HOUR;
