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
#define __SOLAR_CONFIG_H__

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  NEW: For Calibration procedures:
 *  Configuration Defines, testing and production
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Engineer's notes (here one can type the output frequencies from the test to calculate deviation):
// Output at 1.25Hz:		
// Output at 0.5Hz:		
// Factor = Measured-freq / Desired-freq
// Factor = Desired-time / Measured-time
// New-Post-count = Ideal-Post-count * Factor

#define		WDT_PRESC_125HzTest				0b00000000 // 16ms time-out
#define		WDT_PRESC_01HzTest				0b00000101 // 0.5s time-out

#define		TICKS_BEFORE_TOGGLE_125HzTest	25 // original: 25 // 25*0.016 = 0.4 --> 0.4s high, 0.4s low = 1.25Hz
#define		TICKS_BEFORE_TOGGLE_01HzTest	10 // original: 10 // 10*0.5 = 5 --> 5s high, 5s low = 0.1Hz

// With this define commented it'll run prescaled to a 0.1Hz target, uncommented it'll prescale to 1.25Hz
#define		WDT_CALIBRATE_125Hz		1




#define		PINB_WDC_CALIB_TOGGLE	(1<<PINB1) // Can be the LEDPWM or the EnableBoost Pin

/* * * * * * * * * * * * * * *
 *
 *  End of new stuff
 *
 * * * * * * * * * * * * * * */


/*
In the porprietary set up:
PB0/ADC0 -> Sensor
PB1/OC0B -> LED PWM
PB2/CLKO -> Enable LED boost
*/
#define		PORTB_SENSOR_PIN		(1<<PORTB0)
#define		PORTB_LEDPWM_PIN		(1<<PORTB1) // Has to be one of the two PWM outputs
#define		PORTB_ENABLEBOOST_PIN	(1<<PORTB2)

#endif // __SOLAR_COUNTER_H__