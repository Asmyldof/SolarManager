/*
 * SolarCounter_Tiny10.c
 *
 * Created: 6-11-2014 10:08:45
 *  Author: Robert
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Configuration Defines, testing and production
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define		WDT_PRESC_DAY_TESTING			0b00000110 // WDT 1s
#define		WDT_PRESC_NIGHT_TESTING			0b00000101 // WDT 0.5s
#define		TICKS_BEFORE_SAMPLE_TESTING		2

#define		WDT_PRESC_DAY_PRODUCTION		0b00100001 // WDT 8s
#define		WDT_PRESC_NIGHT_PRODUCTION		0b00100000 // WDT 4s
#define		TICKS_BEFORE_SAMPLE_PRODUCTION	15

//#define		USE_PRODUCTION					1

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Configuration Defines, universal
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define		TICK_CONSTANT					630	// The constant from which the day
											 // ticks are subtracted to get the night
											 // ticks. See algorithm document for more
#define		MINIMUM_STREAK					5	// Minimum number of samples in a row
											 // before the system switches over

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
#define		ADC_PRESCALER					0 // ADC prescaler
/*
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
#define		TIMER_PRESCALER					0 // Timer0 Prescaler
/*
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
PB0/ADC0 -> Sensor
PB1/OC0B -> LED PWM
PB2/CLKO -> Enable LED boost
*/
#define		INITIAL_PORTB		0x00					// PORTB at startup
#define		INITIAL_DDRB		(1<<PORTB1|1<<PORTB2)	// DDRB: PB1 and 2 output
#define		INITIAL_DIDR0		(1<<ADC0D)				// Disable input circuitry on ADC0
#define		INITIAL_OCR0B		0x00					// OCR0B at startup





/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Internal Defines
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

#define		WDTCR_VALUE_DAY					(1<<WDE|1<<WDIE)|(WDT_PRESCALER_DAY   & 0b00100111) // Mask out any non-prescaler bits to prevent mistakes
#define		WDTCR_VALUE_NIGHT				(1<<WDE|1<<WDIE)|(WDT_PRESCALER_NIGHT & 0b00100111)

#define		ADCSRA_START					0b11001000 | (ADC_PRESCALER & 0b00000111)

#define		CCP_SIGNATURE					0xD8 // Page 12 of the Datasheets

#define		CLOCK_PRESCALER_INTERNAL		CLOCK_PRESCALER & 0b00001111
#define		SYSTEM_CLOCK_INTERNAL			SYSTEM_CLOCK & 0b00000011
#if SYSTEM_CLOCK_INTERNAL == 3
	#undef SYSTEM_CLOCK_INTERNAL
	#define SYSTEM_CLOCK_INTERNAL	0 // if reserved mode chosen, default back to 8MHz
#endif // SYSTEM_CLOCK_INTERNAL == 3
#if CLOCK_PRESCALER_INTERNAL > 8
	#undef CLOCK_PRESCALER_INTERNAL
	#define CLOCK_PRESCALER_INTERNAL 3 // if reserved mode chosen, default back to Source / 8
#endif

#define		INITIAL_OCR0BL_INTERNAL		INITIAL_OCR0B & 0x0FF
#define		INITIAL_OCR0BH_INTERNAL		INITIAL_OCR0B & 0xFF00

#define		FLAG_SLOWTURNOFF			0x01


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
    /*
		Enable timer
		Enable OCR PWM output
		*/
#if SYSTEM_CLOCK_INTERNAL != 0 // If not the default
	// Set Clock System
	CCP = CCP_SIGNATURE;
	CLKMSR = SYSTEM_CLOCK_INTERNAL;
#endif // SYSTEM_CLOCK_INTERNAL != 0
#if CLOCK_PRESCALER_INTERNAL != 3 // If not the default
#if SYSTEM_CLOCK_INTERNAL == 0
	// Means we did not write CCP yet, so we need to do that here now.
	CCP = CCP_SIGNATURE;
#endif
	// Set Prescaler
	CLKPSR = CLOCK_PRESCALER_INTERNAL;
#endif
	
	OCR0BH = INITIAL_OCR0BH_INTERNAL;
	OCR0BL = INITIAL_OCR0BL_INTERNAL;
	
	
	WDT_CountDown = TICKS_BEFORE_SAMPLE;
	WDTCSR = WDTCR_VALUE_DAY;
	
	sei();
	
	while(1)
    {
		// Infinite empty loop, since the entire program will be interrupt based.
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
	if( OperationalFlags & FLAG_SLOWTURNOFF )
	{
		Temp = OCR0BL;
		if(Temp == 0)
		{
			// TODO: Turn off the enable pin and stop
			
			OperationalFlags &= ~FLAG_SLOWTURNOFF;
		}
		Temp--;
		OCR0BH = 0;
		OCR0BL = Temp;
	}
	
	WDT_CountDown--;
	if(WDT_CountDown <= 1)
	{
		WDT_CountDown = TICKS_BEFORE_SAMPLE;
		ADCSRA = ADCSRA_START;
	}
	
}

/*
  ADC interrupt routine: Is called once every 1min during night-mode and once every 2min
  during day mode. While it is not strictly nescesary to keep checking in night mode, it
  is a good preventative measure in case a glitch occurs, to detect day break and 
  potentially return to normal operation. It may be idle hope, but it's better to  
  over-design the software a little in the hopes of catching some potential break-downs.
*/
ISR(ADC_vect)
{
	/* Test the ADC result:
		When day:	Count a "Tick"(uint16)
					Reset night streak
					if( light = on )
						Count a day streak
						if( daystreak = limit )
							turn off light
							switch to day count timeout
		When night: Reset day streak
					if( light = off )
						count night streak
						if( nightstreak = limit )
							calculate night-ticks
							switch to night count timeout
							enable light
	*/						
	
	
}