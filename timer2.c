/*
 * timer2.c
 *
 * Author: Peter Sutton
 *
 * timer 2 skeleton
 */

#include "timer2.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* Set up timer 2
 */
void init_timer2(void)
{
	//TCNT2 = 0;
	
	/* Set up timer/counter 2 so that we get an 
	** interrupt 100 times per second, i.e. every
	** 10 milliseconds.
	*/
	OCR2A = 255; /* Clock divided by 128 - count for 10000 cycles */
	TCCR2A = (1 << WGM21); /* CTC mode */
	TCCR2B = (1<< CS22)|(1<<CS20); /* Divide clock by 128 */

	/* Enable interrupt on timer on output compare match 
	*/
	TIMSK2 = (1<<OCIE2A);

	/* Ensure interrupt flag is cleared */
	TIFR2 = (1<<OCF2A);
}

