/*
 * SolarCounter_Tiny10.c
 *
 * Created: 6-11-2014 10:08:45
 *  Author: Robert
 */ 


#include <avr/io.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Configuration Defines, testing and production
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define		WDT_PRESC_TESTING				0
#define		TICKS_BEFORE_SAMPLE_TESTING		0

#define		WDT_PRESC_PRODUCTION			0
#define		TICKS_BEFORE_SAMPLE_PRODUCTION	0

//#define		USE_PRODUCTION					1

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Configuration Defines, universal
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define		TICK_CONSTANT					0	// The constant from which the day
											 // ticks are subtracted to get the night
											 // ticks. See algorithm document for more
#define		MINIMUM_STREAK					0	// Minimum number of samples in a row
											 // before the system switches over


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Internal Defines
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef USE_PRODUCTION
#define		WDT_PRESCALER					WDT_PRESC_PRODUCTION
#define		TICKS_BEFORE_SAMPLE				TICKS_BEFORE_SAMPLE_PRODUCTION
#else //USE_PRODUCTION
#define		WDT_PRESCALER					WDT_PRESC_TESTING
#define		TICKS_BEFORE_SAMPLE				TICKS_BEFORE_SAMPLE_TESTING
#endif //USE_PRODUCTION




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  Global variables
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint16_t	Ticks;			// Counter for number of ticks, either samples of day in total or counting down the remaining night ticks
uint8_t		DayStreak;		// Counter for number of day samples in a row
uint8_t		NightStreak;	// Counter for number of night samples in a row

int main(void)
{
    /*
		Enable timer
		Enable OCR PWM output
		Enable WDT Timer
		Enable WDT Interrupt
		Enable ADC
		Enable ADC Interrupt
		Enable General Interrupts
		*/
	
	while(1)
    {
		// Infinite empty loop, since the entire program will be interrupt based.
    }
}




/*
  Interrupt routine for a Watchgod timeout. The watchdog is the basic timing unit for
  the sampling system, where the interrupt is switched over between day and night to
  adjust the 2:1 difference in the algorithm.
*/
ISR(WDT_vect)
{
	/* Count a tick. If tick limit is reached, trigger ADC. Use ticks by decrement
	   to create more efficient code */
	
	/* if night mode end flag is set, decrease OCR0 output, when 0 switch to day
	   count mode */
	
	
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