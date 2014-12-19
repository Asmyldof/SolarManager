/*
 * SolarConfig.h
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
 * environmental temperature. However the user can design their own hardware using the 
 * theory of operation described in the main C file.
 *
 * As it is designed/committed the system will use two minute intervals for the day ticks, and one
 * minute intervals for the night ticks. Combined with a set "DefinedConstant" of around 620 to 640
 * this will create a system that in the winter will keep the lights on to about 23:00 to 23:30 and in 
 * the summer to about 24:00 to 0:45.
 *
 * The following defined constants will be useful to discover the pinning for your own hardware design:
 *   PORTB_LEDPWM_PIN  -- LED PWM Output (Slowly decreases brightness once the time-out is reached)
 *   PORTB_ENABLEBOOST_PIN  -- LED Boost Enable (switches hard on when PWM > 0%, hard off when PWM == 0%)
 *   PORTB_SENSOR_PIN  -- Sensor input (Analog Sensor pin)
 *   ADC_DIDR_SENSOR_PIN  -- ADC Sensor Channel Disable bit - Don't forget to change this when changing the sensor input.
 * 
 * The following values will be useful to you when making a design with a different sensor or
 * supply voltage:
 *   SUPPLY_VOLTAGE_MV
 *   DARK_THRESHOLD_MV
 *   DARK_HYSTERESIS_MV
 *
 * The following values will be helpful if you want to fiddle with the algorithm to get different off-times:
 *   TICK_CONSTANT
 *   TICKS_BEFORE_SAMPLE
 *   WDT_PRESC_NIGHT
 *   WDT_PRESC_DAY
 *   
 * Final Note:
 *    Set or disable the "USE_PRODUCTION" define a few lines below, to go between the two settings of 
 *    time-out. USE_PRODUCTION will go to minute scale, making it measure and run for hours. With
 *    that disabled it will use testing settings, which in the system as it is supplied will be seconds in
 *    stead of minutes, and as such it will run for minutes in stead of hours, allowing the development
 *    and test cycle of tweaked numbers to be executed within 10 to 20 minutes.
 *  
 *    (the USE_PRODUCTION also undefines the DEBUG flag. While this is not used anywhere in this code,
 *    as its presence is heavily dependent on platform and your makefile, I wanted to make sure I
 *    wouldn't get stuck in unintended debug modes in a production unit. This is either decent coding or
 *    sloppy coding, depending on your school of thought. Incidentally, I'm more inclined to find it sloppy,
 *    but it is my experience in public embedded code that most just copy-paste, so I chose safety over
 *    cleanliness.)
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
 * 
 *  Configuration Defines, testing and production
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define		WDT_PRESC_DAY_TESTING			0b00000010 // 64ms + 26 ticks for calibrated // original: 0b00000011 // WDT 0.125s -- Watchdog timer timeout during the day, see WDTCSR in datasheet
#define		WDT_PRESC_NIGHT_TESTING			0b00000001 // 32ms + 26 ticks for calibrated // original: 0b00000010 // WDT 64ms -- Watchdog timer timeout during the night, see WDTCSR in datasheet
#define		TICKS_BEFORE_SAMPLE_DAY_TESTING		26 // number of ticks to count in both situations to get the final sample time-out
#define		TICKS_BEFORE_SAMPLE_NIGHT_TESTING	26

#define		WDT_PRESC_DAY_PRODUCTION		0b00100000 // 4s + 25ticks for calibrated // original: 0b00100001 // WDT 8s
#define		WDT_PRESC_NIGHT_PRODUCTION		0b00000111 // 2s + 25ticks for calibrated // original: 0b00100000 // WDT 4s
#define		TICKS_BEFORE_SAMPLE_DAY_PRODUCTION	25 // original: 15 // The two ticks numbers are dependent on the exact timing 
											 // accuracy and/or offset. In my development device it was off
											 // a little and I needed 14 in stead of 15. Depending on how accurate
											 // you want your system to be you may have to tweak up the numbers
											 // a little here and there. You can get even higher accuracy by going to
											 // WDT time-outs of 2s and 1s, with 60 as initial tick value, but 
											 // I chose slightly decreased accuracy of tweaks over higher energy
											 // savings. (The more often you wake up, the more time you spend 
											 // powered up, using more energy. While compared to 3W of lights
											 // it's still negligible, you never know what might be using up your 
											 // very last Wh of battery).
#define		TICKS_BEFORE_SAMPLE_NIGHT_PRODUCTION	25 // original: 15

//#define		USE_PRODUCTION			// Use this flag to switch between 	testing and production.	

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Configuration Defines, universal
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// TODO: Find the bug in Night Install that makes it hang in light-on mode
// TODO: See if the bug has been fixed (later, when there's time to build a reference example board)
#define		NIGHT_INSTALL					// Setting this define will make the unit start a 120minute 
								// night mode run after power-up, this is most useful when installing at night
								// to see if all the lights are properly connected, plus during the day it'll turn 
								// off after 30 minutes (defined number, can be changed).
#define		NIGHT_INSTALL_TIMEOUT_MINUTES	120

// Make the defined milivolts floating (by addind a trailing .0):
#define		SUPPLY_VOLTAGE_MV				2500.0 // uC supply voltage in mV
#define		DARK_THRESHOLD_MV				450.0 // Level in mV below which it 
											 // is considered dark.
#define		DARK_HYSTERESIS_MV				10.0 // Hysteresis in mV, with a streak longer
											 // than 3 it's less useful to have large
											 // hysteresis.
#define		TICK_CONSTANT					600	// The constant from which the day
											 // ticks are subtracted to get the night
											 // ticks. See algorithm document for more
#define		MINIMUM_NIGHT_STREAK			5	// Minimum number of samples in a row
											 // before the system switches over to night mode
#define		MINIMUM_DAY_STREAK				30 // minimum number of samples to switch to
											 // day mode.
#define		MINIMUM_DAY_BEFORE_NIGHT		300	// Minimum number of minutes counted 
											 // at which the software will accept a streak 
											 // to be actual night time. Shortest day in the
											 // Netherlands is near 8 hours, so I chose
											 // 5 hours = 300 minutes (because maybe
											 // there's leaves falling onto the sensor or
											 // something else like that, never take the
											 // maximum)
#define		MINIMUM_NIGHT_TO_RESET_DAY		40 // Minimum number of consecutive ticks of night 
											 // before the system resets the daystreak
											 // (That is number of ticks that night mode is already
											 //  enabled / light is on. So it will automatically include
											 //  the minimum nightstreak time.)

#define		MINIMUM_AFTERGLOW_MINUTES		120 // Minimum number of ticks to keep the light on, always
#define		MAXIMUM_AFTERGLOW_MINUTES		400 // Maximum number of ticks to keep the light on, always.

// The following defines control at what time the system goes to a lower brightness to conserve energy.
// If this function is to be disabled, either the PWM values can be kept maximum, or the thresholds can be set above the MAXIMUM_AFTERGLOW_MINUTES
#define		AFTERGLOW_LIMITATION_THRESHOLD1	150 // After what number of ticks will the system go to PWM setting one
#define		AFTERGLOW_LIMITATION_THRESHOLD2	270 // After what number of ticks will the system go to PWM setting two

#define		AFTERGLOW_LIMITATION_PWM1		170 // PWM value when Threshold 1 is reached
#define		AFTERGLOW_LIMITATION_PWM2		105 // PWM value when Threshold 2 is reached

#define		SLEEP_MODE						2 // Sleep mode select
/*
NOTE: All modes are available now, but when the PWM output is used by the code, it will automatically go to Idle to keep ClkIO on, to allow PWM
0 - Idle
1 - ADC Noise Reduction
2 - Power-Down
3 - Reserved
4 - Standby
5, 6, 7 - Reserved
*/
#define		SYSTEM_CLOCK					0 // System Clock Source
/*
0 - Calibrated internal 8MHz
1 - Internal WDT 128kHz oscillator
2 - External Clock
3 - Reserved (Will be reverted back to 8MHz)
*/											 
#define		CLOCK_PRESCALER					4 // System Clock Prescaler
/*
0 - Source / 1
1 - Source / 2
2 - Source / 4
3 - Source / 8 (start-up default)
4 - Source / 16
5 - Source / 32
6 - Source / 64
7 - Source / 128
8 - Source / 256
9 and up: reserved (Will be reverted back to Source / 8)
*/
#define		ADC_PRESCALER					2 // ADC prescaler: 
/* Make sure the ADC is between 50kHz and 200kHz
0 - Ckio / 2
1 - Ckio / 2
2 - Ckio / 4
3 - Ckio / 8
4 - Ckio / 16
5 - Ckio / 32
6 - Ckio / 64
7 - Ckio / 128
any higher number is masked back to the above series.
*/
#define		TIMER_PRESCALER					1 // Timer0 Prescaler
/* Can be used in testing to decrease the PWM cycle to something more easily detectable
0 - Timer Turned off
1 - Ckio / 1
2 - Ckio / 8
3 - Ckio / 64
4 - Ckio / 256
5 - Ckio / 1024
6 - External on T0, Falling Edge
7 - External on T0, Rising Edge
*/

/*
In the porprietary set up:
PB0/ADC0 -> Sensor
PB1/OC0B -> LED PWM
PB2/CLKO -> Enable LED boost
*/
#define		PORTB_SENSOR_PIN		(1<<PORTB0)
#define		PORTB_LEDPWM_PIN		(1<<PORTB1) // Has to be one of the two PWM outputs
#define		PORTB_ENABLEBOOST_PIN	(1<<PORTB2)
#define		ADC_DIDR_SENSOR_PIN		(1<<ADC0D)
#define		ADC_ADMUX				0x00			// TODO: Make this and the DIDRPIN tied to the Sensor pin definition through internals
#define		INITIAL_OCR0			0x00			// OCR0 at startup
#define		MAXIMUM_OCR0			0xFF			// In this version, stick to 8 bit
#define		OCR0_DECREASE_STEPSIZE	2 // decrease by 2, so it'll dim over 10 minutes with a 4s WDT interval.
#define		OCR0B_RESOLUTION		8				// Leave this at 8 in this version!
/*
Options: 10, 9 or 8.
If it's not 10, then if it's not 9, it's set to 8.
So any value not 9 or 10 will always be 8 bit.
*/




#endif // __SOLAR_COUNTER_H__