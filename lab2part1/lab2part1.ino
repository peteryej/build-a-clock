#include <PowerDue.h>
#include "DueClock.h" 

void setup() { 
	PowerDue.LED(); 
	PowerDue.LED(PD_OFF); 
	SerialUSB.begin(0); 
	while(!SerialUSB); // wait for serial port to open 
	Clock.init(); 
	Clock.start(); 
 //enable export of the clock on A4
  PIO_Configure(PIOA, PIO_PERIPH_A, PIO_PA2A_TIOA1, PIO_DEFAULT);
} 

void loop() { // get and print time every second
	delay(1000); 
	DueTime_t t; 
	Clock.getTime(&t); 
	SerialUSB.print("Time now is: "); 
	SerialUSB.println(t.sec); 
 } 
 
