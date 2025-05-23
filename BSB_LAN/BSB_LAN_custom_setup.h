// Add custom code for setup function here which will be included at the end of the function

// Set TX Enable pin to output and set high now that the system is initialised and the
// TX line is being driven correctly
pinMode(32, OUTPUT);
digitalWrite(32, HIGH);

// Initialise water meter reader
static_assert(sizeof(everblu_config_t) <= sizeof(custom_eeprom));
everblu_setConfig((everblu_config_t*)&custom_eeprom);
has_everblu = everblu_initialise(custom_longs);
// Save the frequency to MQTT line
if (has_everblu) custom_floats[0] = everblu_getFrequency();
else custom_floats[0] = -1.0f;

