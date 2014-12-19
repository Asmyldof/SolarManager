/*
 * SolarCounter-Tn10-Calibration.c
 *
 * Created: 6-11-2014 10:08:45
 *  Author: Robert van Leeuwen
 *  (c) 2014 Asmyldof, the Netherlands (see notice below)
 *
 * This code is made available under MIT license (see copyright notice below). 
 *
 * The hardware that belongs to it is proprietary, for which the customer paid good money,
 * except for the sensor module design, which is included. The hardware belonging to this code
 * is a heavily structured design to achieve harvesting efficiency of up to 95% and total Solar
 * to battery to light efficiency (overall, all losses included) at or above 75% depending on 
 * environmental temperature. 
 * 
 * This is a WDC calibration project, copied from the main project for testing, to later be
 * included in the main project under a start-up ADC routine using the reset pin.
 * Where reset = VCC means start calibration output, 1kHz, reset = 0.75VCC to 0.95VCC 
 * means start calibration output 1/2Hz. reset < 0.25VCC means normal operation.
 * Later reset between 0.3VCC and 0.5VCC may set night-install mode, allowing an install-time
 * decision on start-up behaviour.
 * 
 * The above scheme allows for calibration operations without resetting the device, after
 * which for normal operation the reset can finally be disabled.
 */ 

/* Copyright Notice:
 * 
 * You are free to use this code in any of your own designs, whether free-ware or not. You are allowed
 * to use it to make buckets and buckets of money. While I would appreciate you pay me a bucket or
 * two if you do, you are in no way obligated. But you might end up with a great help-desk if you do ;-).
 *
 *  ---> But, there's rules! (All rules carry the "Without prior written consent" label, there's always exceptions possible)
 * One: You MUST include this entire notice in the source files that include ANY of my work.
 * Two: Your end-product must contain a reference/dedication to me and preferably my website.
 * Three: Any assistance with any or all of this code may be subject to billing, contact me to find out.
 * Four: You realise that NONE of this code comes with any guarantee when used in your own application
 * Five: You do not use me, my site or my work to promote your own projects using, or not using, this code.
 * Six: You get at least some manner of joy out of using this. Or at least try to.
 *
 *  COPYRIGHT: Robert van Leeuwen, Asmyldof, 2014.
 *                      http://www.asmyldof.com
 *                      git-open@asmyldof.com
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

#include "SolarConfig.h"
#include "SolarCounter.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Global variables
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint8_t		WDT_CountDown;	// Tick Counter (counts down) inside the WDT interrupt

int main(void)
{	
	PORTB = INITIAL_PORTB;
	DDRB = INITIAL_DDRB;
	
	WDTCSR = WDTCR_VALUE_CALIBRATION;

	WDT_CountDown = TICKS_BEFORE_TOGGLE_CALIBRATION; // Counting down in the WDT interrupt: Pre-load!
	
	sei(); // Enable interrupts (very important!)
	
	while(1)
    {
		// in this project: do nothing.
    }
}


/*
  Simple WDT driven Toggle
*/
ISR(WDT_vect)
{	
	// We don't care about WDT Reset Safety (see datasheet), so we can just re-enable here:
	WDTCSR |= (1<<WDIE);
	
	WDT_CountDown--;
	if(WDT_CountDown == 0)
	{
		WDT_CountDown = TICKS_BEFORE_TOGGLE_CALIBRATION;
		PINB = PINB_WDC_CALIB_TOGGLE; // Toggle the preset output
	}
}

/* ADC not used in calibration project */
ISR(ADC_vect)
{
	
}