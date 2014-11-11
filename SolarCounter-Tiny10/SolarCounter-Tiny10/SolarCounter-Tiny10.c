/*
 * SolarCounter_Tiny10.c
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
 * theory of operation described below.
 * 
 * This file contains a configurable solar-power checker with an intelligent system to determine
 * light hours and calculate dark hours before midnight. Allowing the system to switch on the
 * lights when the sun is setting (or has set) and then continue to keep the lights on for a certain
 * amount of time, determined by a preset constant and the number of detected ticks of day.
 *
 * It is aimed at using an SFH325FA by OSRAM with a 15kOhm resistor to ground.
 * Another sensor can be used, but some research may be needed to determine the operation
 * in solar cycles and then set the light threshold values. The SFH325FA has the advantage of
 * having a great immunity for non-infrared wavelengths, eliminating most artificial light sources
 * and giving no response at all to lunar light.
 *
 * The program is created for a system using a 4W to 10W solar panel to achieve reasonable
 * power harvesting in winter days, a 80Wh+ 3.7V Lithium-Polymer battery and a boost 
 * boost converter for the lights with an independent enable, allowing extreme power-saving
 * when not having the lights enabled.
 * 
 * Theory of operation:
 * The system uses the WDT interrupt as a timing mechanism, where the time-out is switched 
 * to create a difference between timing of day-light samples and night-time samples.
 *
 * When the system sees sunshine (or day-time light at least)  it counts a tick, this happens every
 * two minutes once day-time has initiated (since night-time time-out is different a slight offset
 * may occur at some instances, but this is okay, since on the grand scheme it doesn't differ that
 * much, no need to over-complicate the software).
 *
 * After the system sees a minimum series of samples of no-sun-light (no IR present), it switches
 * to night mode, first it calculates a new number of ticks:
 * Ticks = DefinedConstant - PreviouslyCountedDayTicks.
 * This way, if the days are short, the new Ticks number will be higher, than when the days are long.
 * Then, every time the system sees a night value, it will decrease the Ticks variable. When it hits 0
 * it will start to decrease the PWM value on the LEDoutput over a configurable number of minutes.
 *
 * As it is designed/committed the system will use two minute intervals for the day ticks, and one
 * minute intervals for the night ticks. Combined with a set "DefinedConstant" of aroun 620 to 640
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Configuration Defines, testing and production
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define		WDT_PRESC_DAY_TESTING			0b00000011 // WDT 64ms -- Watchdog timer timeout during the day, see WDTCSR in datasheet
#define		WDT_PRESC_NIGHT_TESTING			0b00000010 // WDT 0.128s -- Watchdog timer timeout during the night, see WDTCSR in datasheet
#define		TICKS_BEFORE_SAMPLE_TESTING		14 // number of ticks to count in both situations to get the final sample time-out

#define		WDT_PRESC_DAY_PRODUCTION		0b00100001 // WDT 8s
#define		WDT_PRESC_NIGHT_PRODUCTION		0b00100000 // WDT 4s
#define		TICKS_BEFORE_SAMPLE_PRODUCTION	15 // The two ticks numbers are dependent on the exact timing 
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

#define		USE_PRODUCTION			// Use this flag to switch between 	testing and production.	

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Configuration Defines, universal
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// TODO: Find the bug in Night Install that makes it hang in light-on mode
// TODO: See if the bug has been fixed (later, when there's time to build a reference example board)
//#define		NIGHT_INSTALL					// Setting this define will make the unit start a 120minute 
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
#define		TICK_CONSTANT					640	// The constant from which the day
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
#define		MINIMUM_AFTERGLOW_MINUTES		130 // Minimum number of ticks to keep the light on, always
#define		MAXIMUM_AFTERGLOW_MINUTES		415 // Maximum number of ticks to keep the light on, always.

#define		SLEEP_MODE						4 // Sleep mode select -- 4 is good for all, 2 for production only
/*
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



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Internal Defines - Do not tamper with! Use the defines above to change parameters
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef USE_PRODUCTION
#define		WDT_PRESCALER_DAY				WDT_PRESC_DAY_PRODUCTION
#define		WDT_PRESCALER_NIGHT				WDT_PRESC_NIGHT_PRODUCTION
#define		TICKS_BEFORE_SAMPLE				TICKS_BEFORE_SAMPLE_PRODUCTION
#ifdef DEBUG
#undef DEBUG
#endif //DEBUG
#else //USE_PRODUCTION
#define		WDT_PRESCALER_DAY				WDT_PRESC_DAY_TESTING
#define		WDT_PRESCALER_NIGHT				WDT_PRESC_NIGHT_TESTING
#define		TICKS_BEFORE_SAMPLE				TICKS_BEFORE_SAMPLE_TESTING
#ifndef		DEBUG
#define		DEBUG
#endif		//DEBUG
#endif //USE_PRODUCTION

#define		WDTCR_VALUE_DAY					(1<<WDIE)|(WDT_PRESCALER_DAY   & 0b00100111) // Mask out any non-prescaler bits to prevent mistakes
#define		WDTCR_VALUE_NIGHT				(1<<WDIE)|(WDT_PRESCALER_NIGHT & 0b00100111)

#define		ADCSRA_START					0b11001000 | (ADC_PRESCALER & 0b00000111)

#define		CCP_SIGNATURE					0xD8 // Page 12 of the Datasheets

// Nest two defines include brackets specifically for logic pre-proc tests later on, don't remove them.
#define		CLOCK_PRESCALER_INTERNAL		(CLOCK_PRESCALER & 0b00001111)
#define		SYSTEM_CLOCK_INTERNAL			(SYSTEM_CLOCK & 0b00000011)

#if (SYSTEM_CLOCK_INTERNAL == 3)
#warning "System Clock Definition set to reserved state: Reverting to 8MHz internal clock"
	#undef SYSTEM_CLOCK_INTERNAL
	#define SYSTEM_CLOCK_INTERNAL	0 // if reserved mode chosen, default back to 8MHz
#endif // SYSTEM_CLOCK_INTERNAL == 3
#if (CLOCK_PRESCALER_INTERNAL > 8)
#warning "Clock Prescaler Set to reserved state: Reverting to Source / 8 (default)"
	#undef CLOCK_PRESCALER_INTERNAL
	#define CLOCK_PRESCALER_INTERNAL 3 // if reserved mode chosen, default back to Source / 8
#endif

#define		INITIAL_OCR0L_INTERNAL		INITIAL_OCR0 & 0x0FF
#define		INITIAL_OCR0H_INTERNAL		INITIAL_OCR0 & 0xFF00

#define		MAXIMUM_OCR0L_INTERNAL		MAXIMUM_OCR0 & 0x0FF
#define		MAXIMUM_OCR0H_INTERNAL		MAXIMUM_OCR0 & 0xFF00

// Calculate the dark and light thresholds from the values set above:
#define		DARK_THRESHOLD		(uint8_t)(((DARK_THRESHOLD_MV - DARK_HYSTERESIS_MV)*255.0)/SUPPLY_VOLTAGE_MV)
#define		LIGHT_THRESHOLD		(uint8_t)(((DARK_THRESHOLD_MV + DARK_HYSTERESIS_MV)*255.0)/SUPPLY_VOLTAGE_MV)

#define		MINIMUM_DAY_BEFORE_NIGHT_INTERNAL	(MINIMUM_DAY_BEFORE_NIGHT >> 1) // divide by two
													// TODO: Make the above shift more flexible toward
													// different WDT interrupt distances


// Define PORTB and DDRB from defined pins:
#define		INITIAL_PORTB		0x00					// PORTB at startup
#define		INITIAL_DDRB		PORTB_LEDPWM_PIN|PORTB_ENABLEBOOST_PIN
														// DDRB: LEDEnable & LEDPWM output
#define		INITIAL_DIDR0		ADC_DIDR_SENSOR_PIN		// Disable input circuitry on SENSOR

#if OCR0B_RESOLUTION == 10
#warning "PWM Resolution set to 10 bits, please check if this is intended"
#error "10bit resoltion not supported yet"
// Even though the code doesn't support it yet, the TCCR0A is added here, so when it's
// supportes, only the error needs to be removed.
#define		TCCR0A_INTERNAL		0b00100011
// TODO: WDT interrupt needs to be made compatible with 10 and 9 bit PWM before the above error is removed!
#elif OCR0B_RESOLUTION == 9
#warning "PWM Resolution set to 9 bits, please check if this is intended"
#error "9bit resoltion not supported yet"
// Even though the code doesn't support it yet, the TCCR0A is added here, so when it's
// supportes, only the error needs to be removed.
#define		TCCR0A_INTERNAL		0b00100010
// TODO: WDT interrupt needs to be made compatible with 10 and 9 bit PWM before the above error is removed!
#else // OCR0B_RESOLUTION is not 10 or 9
#define		TCCR0A_INTERNAL		0b00100001
#endif
#define		TCCR0B_INTERNAL		0x08|(TIMER_PRESCALER & 0x07)

#if	(PORTB_LEDPWM_PIN == (1<<PORTB1))
#define		OCR0OUT_REGISTER_LOW	OCR0BL
#define		OCR0OUT_REGISTER_HIGH	OCR0BH
#elif (PORTB_LEDPWM_PIN == (1<<PORTB0))
#define		OCR0OUT_REGISTER_LOW	OCR0AL
#define		OCR0OUT_REGISTER_HIGH	OCR0AH
#else
#error "Configured LEDPWM output pin is not a Timer0 PWM enabled pin."
#endif

#define		ACSR_INTERNAL				0x80 // Power disable to the Analog Comparator.
#define		PRR_TIMEROFF				0x01 // PRR value to disable the Timer

#if (SLEEP_MODE == 3)
#warning "Reserved Sleep Mode selected, reverting to Idle mode (0)."
	#undef SLEEP_MODE
	#define SLEEP_MODE	0
#elif (SLEEP_MODE > 4)
#warning "Reserved Sleep Mode selected, reverting to Idle mode (0)."
	#undef SLEEP_MODE
	#define SLEEP_MODE	0
#endif

#define		SMCR_INTERNAL				(SLEEP_MODE << 1)|0x01 // Shift up sleep mode and add enable bit by logic or


#define		FLAG_SLOWTURNOFF			0x01
#define		FLAG_LASTMODE_WAS_DAY		0x02
#define		FLAG_LIGHTISON				0x04
#define		FLAG_SET_SLEEP				0x08



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Local inline helper functions (see end of file):
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

inline static void SwitchToDayMode();
inline static void SwitchToNightMode();


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Global variables
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint16_t	Ticks;			// Counter for number of ticks, either samples of day in total or counting down the remaining night ticks
uint8_t		DayStreak;		// Counter for number of day samples in a row
uint8_t		NightStreak;	// Counter for number of night samples in a row
uint8_t		WDT_CountDown;	// Tick Counter (counts down) inside the WDT interrupt
uint8_t		OperationalFlags; // Flag Register

int main(void)
{
	// Disable the power to the Analog Comparator:
	ACSR = ACSR_INTERNAL;
	
	// Set protected registers, if required:
#if (SYSTEM_CLOCK_INTERNAL != 0) // If not the default
#warning "Reconfiguring the system clock source. Please make sure this is intended."
	// Set Clock System
	CCP = CCP_SIGNATURE;
	CLKMSR = SYSTEM_CLOCK_INTERNAL;
#endif // SYSTEM_CLOCK_INTERNAL != 0
#if (CLOCK_PRESCALER_INTERNAL != 3) // If not the default
#warning "Reconfiguring the system prescaler. Please make sure this is intended."
#if SYSTEM_CLOCK_INTERNAL == 0
	// Means we did not write CCP yet above, so we need to do that here now.
	CCP = CCP_SIGNATURE;
#endif
	// Set Prescaler
	CLKPSR = CLOCK_PRESCALER_INTERNAL;
#endif
	
	SMCR = SMCR_INTERNAL;
	
	PORTB = INITIAL_PORTB;
	DDRB = INITIAL_DDRB;
	DIDR0 = INITIAL_DIDR0;
	ADMUX = ADC_ADMUX;
	
#ifdef	NIGHT_INSTALL
	// TODO: Find out if this has been fixed by moving Ticks = 0; inside Else :-)
	SwitchToNightMode();
	OperationalFlags &= ~FLAG_LASTMODE_WAS_DAY;
	Ticks = NIGHT_INSTALL_TIMEOUT_MINUTES;
#else	
	SwitchToDayMode();
	Ticks = 0; // Make sure we start at 0 ticks, since that's safest.
#endif

	WDT_CountDown = TICKS_BEFORE_SAMPLE; // Counting down in the WDT interrupt: Pre-load!
	
	sei(); // Enable interrupts (very important!)
	
	while(1)
    {
		
		if( ( OperationalFlags & FLAG_SET_SLEEP) == FLAG_SET_SLEEP )
		{
			// We have to enter sleep here, to avoid interrupt collisions when waking up
			OperationalFlags &= ~FLAG_SET_SLEEP;
			sleep_cpu();
		}
		
    }
}




/*
  Interrupt routine for a Watchdog timeout. The watchdog is the basic timing unit for
  the sampling system, where the interrupt is switched over between day and night to
  adjust the 2:1 difference in the algorithm.
*/
ISR(WDT_vect)
{
	uint8_t	Temp;
	
	/* Count a tick. If tick limit is reached, trigger ADC. Use ticks by decrement
	   to create more efficient code */
	/* if night mode end flag is set, decrease OCR0 output, when 0 switch to day
	   count mode */
	
	if( (OperationalFlags & FLAG_SLOWTURNOFF) == FLAG_SLOWTURNOFF )
	{
		//TODO: Make compatible with higher resolution PWM
		Temp = OCR0OUT_REGISTER_LOW;
		if(Temp <= OCR0_DECREASE_STEPSIZE) // this is a test to see if we're within the end-margin.
		{                 // since step size can be arbitrary just checking for 0 can become an infinite
						  // loop. For example with step size 2, it will wrap around once, going from
						  // 1 to 254, skipping 0 the first go around. While with this check, the 
						  // system will switch to off/day-mode when the value is 1.
			SwitchToDayMode();
			Ticks = 0; // Reset the Ticks buffer to make sure we start fresh again, though this should
			          // be guaranteed
		}
		else
		{ // If we haven't reached the end of dimming yet; decrease the PWM value by 1 defined step size:
			Temp -= OCR0_DECREASE_STEPSIZE; 
			OCR0OUT_REGISTER_HIGH = 0; //TODO: Make compatible with higher resolution PWM
			OCR0OUT_REGISTER_LOW = Temp;
		}
	}
	
	// We don't care about WDT Reset Safety (see datasheet), so we can just re-enable here:
	WDTCSR |= (1<<WDIE);
	
	// In all cases: continue running the ADC module to sample day or night to determine further action
	WDT_CountDown--;
	if(WDT_CountDown == 0)
	{ // This little bit is a small post-scaler of course, allowing a 1 or 2 minute interval.
		WDT_CountDown = TICKS_BEFORE_SAMPLE;
		ADCSRA = ADCSRA_START;
	}
	else
	{
		// If not starting an ADC conversion: Go back to sleep mode through the main routine:
		OperationalFlags |= FLAG_SET_SLEEP;
		// Because we want the option of "deep sleep", we need to only trigger when no
		// ADC is started, as the ADC will not run in Stand-By and Power Down
	}
}

/*
  ADC interrupt routine: Is called once every 1min during night-mode and once every 2min
  during day mode. While it is not strictly necessary to keep checking in night mode, it
  is a good preventative measure in case a glitch occurs, to detect day break and 
  potentially return to normal operation. It may be idle hope, but it's better to  
  over-design the software a little in the hopes of catching some potential break-downs.
*/
ISR(ADC_vect)
{
	uint8_t	Temp;
	
	/* Test the ADC result:
		When day:	Count a "Tick"(uint16)
					Reset night streak
					if( light = on )
						Count a day streak
						if( daystreak = limit )
							turn off light
							switch to day count timeout
		When night:	Reset day streak
					if( light = off )
						count night streak
						if( nightstreak = limit )
							calculate night-ticks
							switch to night count timeout
							enable light
					else
						decrease tick
							if( tick == 0 )
								set decrease flag
	*/						
	Temp = ADCL;
	
	if( Temp > LIGHT_THRESHOLD )
	{ // When Day:
		Ticks++; // count a tick
		NightStreak = 0; // reset night streak
		
		DayStreak++; // add day streak
		if( DayStreak >= MINIMUM_DAY_STREAK )
		{
			// switch to day mode
			SwitchToDayMode();
			OperationalFlags |= FLAG_LASTMODE_WAS_DAY; // Set previous mode to day, so night mode can be triggered
			DayStreak = 0; // May as well reset the day streak, since we don't need it anymore
				// code cleanliness.
		}
		
	}
	else if( Temp < DARK_THRESHOLD )
	{ // When night:
		DayStreak = 0; // reset day streak
		if( (OperationalFlags & FLAG_LIGHTISON) != FLAG_LIGHTISON )
		{ // if light is off
			
			// introduced a new flag to prevent repeated triggering of night mode
			// after turning off the lights when the time-out is reached:
			if( (OperationalFlags & FLAG_LASTMODE_WAS_DAY) == FLAG_LASTMODE_WAS_DAY)
			{
				NightStreak++; 
				if( (NightStreak >= MINIMUM_NIGHT_STREAK) && (Ticks >= MINIMUM_DAY_BEFORE_NIGHT_INTERNAL) )
				{ // If the nightstreak is long enough and there were plenty Ticks:
										
					if(Ticks >= TICK_CONSTANT)
					{
						Ticks = MINIMUM_AFTERGLOW_MINUTES; // Cap the calculation to prevent overruns
					}
					else
					{
						Ticks = TICK_CONSTANT - Ticks; // Calculate the night-ticks.
						
						if(Ticks < MINIMUM_AFTERGLOW_MINUTES)
						{
							Ticks = MINIMUM_AFTERGLOW_MINUTES; // Again cap to minimum
						}
						else if(Ticks > MAXIMUM_AFTERGLOW_MINUTES)
						{
							Ticks = MAXIMUM_AFTERGLOW_MINUTES; // And cap to a maximum
						}
					}
					
					SwitchToNightMode();
					OperationalFlags &= ~FLAG_LASTMODE_WAS_DAY; // make sure we don't re-trigger.
					NightStreak = 0;
				}
			}
		}
		else
		{ // if in stead the light is on:
			Ticks--; // take off one tick
			if( Ticks == 0 )
			{
				OperationalFlags &= ~FLAG_LIGHTISON; // prevent "light on" events from triggering
				OperationalFlags |= FLAG_SLOWTURNOFF; // enable PWM slow decrease.
			}
		}
	}
	
	// When the ADC is done, go into sleep (since the WDT will interrupt it again:
	OperationalFlags |= FLAG_SET_SLEEP;
}

// helper function: Switch to day mode: Turn off lights, set timer to power saving, set WDT sampling to day interval
inline static void SwitchToDayMode()
{
	TCCR0B = 0x00; // turn timer off
	TCCR0A = 0x00; // turn timer off
	OCR0OUT_REGISTER_HIGH = INITIAL_OCR0H_INTERNAL;
	OCR0OUT_REGISTER_LOW = INITIAL_OCR0L_INTERNAL;
	PORTB &= ~PORTB_ENABLEBOOST_PIN; // turn off the booster 
	WDTCSR = WDTCR_VALUE_DAY; // switch to day interval
	PRR |= PRR_TIMEROFF; // Turn off the timer module to save energy when in day mode.
	OperationalFlags &= ~(FLAG_SLOWTURNOFF | FLAG_LIGHTISON);
}

// helper function: Switch to night mode: Turn on lights, enable timer, set WDT sampling to night interval
inline static void SwitchToNightMode()
{
	PRR &= ~PRR_TIMEROFF; // Turn on the timer module to enable PWM.
	PORTB |= PORTB_ENABLEBOOST_PIN; // turn on LED boost
	WDTCSR = WDTCR_VALUE_NIGHT; // switch to night interval
	OCR0OUT_REGISTER_HIGH = MAXIMUM_OCR0H_INTERNAL;
	OCR0OUT_REGISTER_LOW = MAXIMUM_OCR0L_INTERNAL;
	TCCR0B = TCCR0B_INTERNAL; // Enable timer functionality
	TCCR0A = TCCR0A_INTERNAL;
	OperationalFlags &= ~FLAG_SLOWTURNOFF; // No slow turn off, since we just started night mode
	OperationalFlags |= FLAG_LIGHTISON; // Set light on flag in the flagbyte
}
