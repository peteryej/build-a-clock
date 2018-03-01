#include <Arduino.h>

#if defined(_SAM3XA_)

#include "DueClock.h"

uint32_t Rc = (VARIANT_MCK/32);
uint32_t newRc = Rc;

void DueClock::init(){
  //diable register write protect to allow TC configuration
	pmc_set_writeprotect(false);

	//configure a 1 Hz clock on TC0-Ch1
	//enable TC0-Ch1 peripheral clock
	pmc_enable_periph_clk(TC1_IRQn);

	//configure the channel mode register to waveform mode
	TC_Configure(TC0, 1,
		TC_CMR_WAVE |                    //set tc waveform mode
		TC_CMR_WAVSEL_UP_RC |            //count up with automatic reset when reaching RC value
		TC_CMR_TCCLKS_TIMER_CLOCK3 |     // use timer clock3, MCK/32 ~2.625 MHz
		TC_CMR_ACPA_CLEAR |				 // set TIOA to low when counter value reaches RA
		TC_CMR_ACPC_SET					 // set TIOA to high when counter value reaches RC
	);

	// configure the values of RC and RA registers
	// how many ticks should we count in one cycle to achieve a 1 Hz clock
	// VARIANT_MCK holds the value of the MCK on this board
	//uint32_t Rc = (VARIANT_MCK/32); //count this many ticks in one second
	TC_SetRC(TC0, 1, Rc);
	TC_SetRA(TC0, 1, Rc/2);          // set RA to be half of RC to achieve 50% duty cycle

	//enable RA and RC compare Interrupt
	TC0->TC_CHANNEL[1].TC_IER = (TC_IER_CPAS | TC_IER_CPCS);
	//disable all other interrupts
	TC0->TC_CHANNEL[1].TC_IDR = ~(TC_IER_CPAS | TC_IER_CPCS);

	//enable TC0-Ch0 peripheral clock
	pmc_enable_periph_clk(TC0_IRQn);

	//configure the channel mode register to waveform mode
	TC_Configure(TC0, 0,
		TC_CMR_WAVE |                    //set tc waveform mode
		TC_CMR_WAVSEL_UP |               //count up till 2^32
		TC_CMR_TCCLKS_XC0 				       // use XC0 as input
	);
	//configure XC0 to use TIOA1 as input
	TC0->TC_BMR = TC_BMR_TC0XC0S_TIOA1;


	//clear any pending interrupts
	NVIC_ClearPendingIRQ(TC1_IRQn);
	//enable the interrupt vector
	NVIC_EnableIRQ(TC1_IRQn);

}

void DueClock::start(){
  TC_Start(TC0,1);
  TC_Start(TC0,0);
}

void DueClock::stop(){
  TC_Stop(TC0,1);
  TC_Stop(TC0,0);
}

void DueClock::addOffset(diffTime_t *offset){
  _secondsOffset += offset->sec;
 
  newRc = (uint32_t)((1000000 - offset->usec)*2.625);

}

void DueClock::reset(){
  this->start(); //calling start again will reset counters
}

void DueClock::getTime(DueTime_t *t){
	
	t->usec = (double)TC_ReadCV(TC0,1)/2.625;
  //t->sec = TC_ReadCV(TC0,0) + _offset.sec;
  t->sec = _seconds +_secondsOffset;
}

void DueClock::tick(){
	_seconds ++;
}

void TC1_Handler(void){ 
	// you must always read the approriate status register when an interrupt happens 
	// or else, your system will keep triggering an interrupt and not allow other 
	// processing events to proceed. 
	// Reading the status register clears the interrupt bit 
	uint32_t status = TC_GetStatus(TC0, 1); 

	// determine if we've reached the RA value or the RC value 
	// LED will blink along with the 1 Hz square wave 
	if(status & TC_SR_CPAS){  // counter has reached RA value 
		//PowerDue.LED(PD_OFF); 
	} else if(status & TC_SR_CPCS){ // counter has reached RC value 
		//PowerDue.LED(PD_RED); 
    TC_SetRC(TC0, 1, newRc);
    TC_SetRA(TC0, 1, newRc/2);
    newRc = Rc;
		// when this happens, we should increment our usecond counter 
    
		Clock.tick(); 
	} 
} 


DueClock Clock = DueClock();

#endif
