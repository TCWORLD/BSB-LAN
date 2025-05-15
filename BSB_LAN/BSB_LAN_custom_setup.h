// Add custom code for setup function here which will be included at the end of the function

// Set TX Enable pin to output and set high now that the system is initialised and the
// TX line is being driven correctly
pinMode(32, OUTPUT);
digitalWrite(32, HIGH);

