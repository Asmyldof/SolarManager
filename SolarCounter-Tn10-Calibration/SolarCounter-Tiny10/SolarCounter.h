/*
 * SolarConfig.h
 *
 * Created: 6-11-2014 10:08:45
 *  Author: Robert van Leeuwen
 *  (c) 2014 Asmyldof, the Netherlands (see notice below)
 *
 * This code is made available under MIT license (see copyright notice below). 
 *
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


#ifndef __SOLAR_COUNTER_H__
#define __SOLAR_COUNTER_H__

#include "SolarConfig.h" // Get the configuration defines in here to be able to do the calculations below;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  NEW: For Calibration procedures:
 *  Internal Defines - Do not tamper with! Use the defines above to change parameters
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define		WDT_PRESC_1kHzTest
#define		WDT_PRESC_05HzTest

#define		TICKS_BEFORE_TOGGLE_1kHzTest
#define		TICKS_BEFORE_TOGGLE_05HzTest


#ifdef WDT_CALIBRATE_125Hz
	#define WDT_PRESC_CALIBRATION			WDT_PRESC_125HzTest
	#define TICKS_BEFORE_TOGGLE_CALIBRATION	TICKS_BEFORE_TOGGLE_125HzTest
#else // WDT_CALIBRATE_1kHz
	#define WDT_PRESC_CALIBRATION			WDT_PRESC_01HzTest
	#define TICKS_BEFORE_TOGGLE_CALIBRATION	TICKS_BEFORE_TOGGLE_01HzTest
#endif // WDT_CALIBRATE_1kHz

#define		WDTCR_VALUE_CALIBRATION			(1<<WDIE)|(WDT_PRESC_CALIBRATION & 0b00100111) // Mask out any non-prescaler bits to prevent mistakes


// Define PORTB and DDRB from defined pins:
#define		INITIAL_DDRB		PORTB_LEDPWM_PIN|PORTB_ENABLEBOOST_PIN
														// DDRB: LEDEnable & LEDPWM output
#define		INITIAL_PORTB		(INITIAL_DDRB &	~PINB_WDC_CALIB_TOGGLE)
						// PORTB at startup: Set the pin that's not used for toggle on, so calibration 
						// will toggle the lights even in the final setup, to allow stop-watching the 0.1Hz signal


#endif // __SOLAR_COUNTER_H__