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
 * Configurations can be changed in SolarConfig.h
 *
 * CHANGING THE DEFINES AND SET-UPs IN THIS FILE IS NOT PART OF GENERAL CONFIGURATION!
 *       ANY CHANGE MADE HERE SHOULD BE PART OF A DESIGN CONSIDERATION!
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
 * 
 *  Internal Defines - Do not tamper with! Use the defines above to change parameters
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef USE_PRODUCTION
#define		WDT_PRESCALER_DAY				WDT_PRESC_DAY_PRODUCTION
#define		WDT_PRESCALER_NIGHT				WDT_PRESC_NIGHT_PRODUCTION
#define		TICKS_BEFORE_SAMPLE_DAY			TICKS_BEFORE_SAMPLE_DAY_PRODUCTION
#define		TICKS_BEFORE_SAMPLE_NIGHT		TICKS_BEFORE_SAMPLE_NIGHT_PRODUCTION
#ifdef DEBUG
#undef DEBUG
#endif //DEBUG
#else //USE_PRODUCTION
#define		WDT_PRESCALER_DAY				WDT_PRESC_DAY_TESTING
#define		WDT_PRESCALER_NIGHT				WDT_PRESC_NIGHT_TESTING
#define		TICKS_BEFORE_SAMPLE_DAY			TICKS_BEFORE_SAMPLE_DAY_TESTING
#define		TICKS_BEFORE_SAMPLE_NIGHT		TICKS_BEFORE_SAMPLE_NIGHT_TESTING
#ifndef		DEBUG
#define		DEBUG
#endif		//DEBUG
#endif //USE_PRODUCTION

#define		WDTCR_VALUE_DAY					((1<<WDIE)|(WDT_PRESCALER_DAY   & 0b00100111)) // Mask out any non-prescaler bits to prevent mistakes
#define		WDTCR_VALUE_NIGHT				((1<<WDIE)|(WDT_PRESCALER_NIGHT & 0b00100111))

#define		ADCSRA_START					(0b11001000 | (ADC_PRESCALER & 0b00000111))

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

#define		INITIAL_OCR0L_INTERNAL		(INITIAL_OCR0 & 0x0FF)
#define		INITIAL_OCR0H_INTERNAL		(INITIAL_OCR0 & 0xFF00)

#define		MAXIMUM_OCR0L_INTERNAL		(MAXIMUM_OCR0 & 0x0FF)
#define		MAXIMUM_OCR0H_INTERNAL		(MAXIMUM_OCR0 & 0xFF00)

// Calculate the dark and light thresholds from the values set above:
#define		DARK_THRESHOLD		(uint8_t)(((DARK_THRESHOLD_MV - DARK_HYSTERESIS_MV)*255.0)/SUPPLY_VOLTAGE_MV)
#define		LIGHT_THRESHOLD		(uint8_t)(((DARK_THRESHOLD_MV + DARK_HYSTERESIS_MV)*255.0)/SUPPLY_VOLTAGE_MV)

#define		MINIMUM_DAY_BEFORE_NIGHT_INTERNAL	(MINIMUM_DAY_BEFORE_NIGHT >> 1) // divide by two
													// TODO: Make the above shift more flexible toward
													// different WDT interrupt distances


// Define PORTB and DDRB from defined pins:
#define		INITIAL_PORTB		0x00					// PORTB at startup
#define		INITIAL_DDRB		(PORTB_LEDPWM_PIN | PORTB_ENABLEBOOST_PIN)
														// DDRB: LEDEnable & LEDPWM output
#define		INITIAL_DIDR0		ADC_DIDR_SENSOR_PIN		// Disable input circuitry on SENSOR

#if OCR0B_RESOLUTION == 10
#warning "PWM Resolution set to 10 bits, please check if this is intended"
#error "10bit resoltion not supported yet"
// Even though the code doesn't support it yet, the TCCR0A is added here, so when it's
// supportes, only the error needs to be removed.
#define		TCCR0A_PRE_INTERNAL		0b00000011
// TODO: WDT interrupt needs to be made compatible with 10 and 9 bit PWM before the above error is removed!
#elif OCR0B_RESOLUTION == 9
#warning "PWM Resolution set to 9 bits, please check if this is intended"
#error "9bit resoltion not supported yet"
// Even though the code doesn't support it yet, the TCCR0A is added here, so when it's
// supportes, only the error needs to be removed.
#define		TCCR0A_PRE_INTERNAL		0b00000010
// TODO: WDT interrupt needs to be made compatible with 10 and 9 bit PWM before the above error is removed!
#else // OCR0B_RESOLUTION is not 10 or 9
#define		TCCR0A_PRE_INTERNAL		0b00000001
#endif
#define		TCCR0B_INTERNAL		(0x08|(TIMER_PRESCALER & 0x07))

#if	(PORTB_LEDPWM_PIN == (1<<PORTB1))
	#define		OCR0OUT_REGISTER_LOW	OCR0BL
	#define		OCR0OUT_REGISTER_HIGH	OCR0BH
	#define		TCCR0A_OCR_BITS			0b00100000
#elif (PORTB_LEDPWM_PIN == (1<<PORTB0))
	#define		OCR0OUT_REGISTER_LOW	OCR0AL
	#define		OCR0OUT_REGISTER_HIGH	OCR0AH
	#define		TCCR0A_OCR_BITS			0b10000000
#else
#error "Configured LEDPWM output pin is not a Timer0 PWM enabled pin."
#endif

// Build up the TCCR0A value using previously made conditional defines:
#define		TCCR0A_INTERNAL			(TCCR0A_PRE_INTERNAL | TCCR0A_OCR_BITS)

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


#define		SMCR_INTERNAL_LOWEST_ALLOWED	((SLEEP_MODE << 1)|0x01) // Shift up sleep mode and add enable bit by logic or

#if (SLEEP_MODE == 0) // If the sleep mode is set to idle, with ClkIO enabled, set the undifferentiated flag:
#define		SMCR_UNDIFFERENTIATED
#endif

// For PWM state, enable sleep mode, with IDLE forced to keep ClkIO running for PWM:
#define		SMCR_INTERNAL_AT_PWM			0x01 // Enable sleep, with Idle mode forced.

// If the wrong threshold is higher (if 1 is higher than 2), swap all the limitation values: 
#if (AFTERGLOW_LIMITATION_THRESHOLD1 > AFTERGLOW_LIMITATION_THRESHOLD2) 
#define		AFTERGLOW_LIMITATION_THRESHOLD1_INTERNAL	AFTERGLOW_LIMITATION_THRESHOLD2
#define		AFTERGLOW_LIMITATION_THRESHOLD2_INTERNAL	AFTERGLOW_LIMITATION_THRESHOLD1
#define		AFTERGLOW_LIMITATION_PWM1_INTERNAL			AFTERGLOW_LIMITATION_PWM2
#define		AFTERGLOW_LIMITATION_PWM2_INTERNAL			AFTERGLOW_LIMITATION_PWM1
#warning "AFTERGLOW_LIMITATION_THRESHOLD1 larger than THRESHOLD2, swapping 1 and 2 values."
#else
#define		AFTERGLOW_LIMITATION_THRESHOLD1_INTERNAL	AFTERGLOW_LIMITATION_THRESHOLD1
#define		AFTERGLOW_LIMITATION_THRESHOLD2_INTERNAL	AFTERGLOW_LIMITATION_THRESHOLD2
#define		AFTERGLOW_LIMITATION_PWM1_INTERNAL			AFTERGLOW_LIMITATION_PWM1
#define		AFTERGLOW_LIMITATION_PWM2_INTERNAL			AFTERGLOW_LIMITATION_PWM2
#endif

#define		FLAG_SLOWTURNOFF			0x01
#define		FLAG_LASTMODE_WAS_DAY		0x02
#define		FLAG_LIGHTISON				0x04
#define		FLAG_SET_SLEEP				0x08
#define		FLAG_RUNNING_DAY			0x10
#define		FLAG_PWM_OPERATONAL			0x20

#endif // __SOLAR_COUNTER_H__