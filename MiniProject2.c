/*
 * MiniProject2.c
 *
 *  Created on: Sep 17, 2022
 *      Author: Naiera Seifeldin
 */



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/******************* global variables for stop watch time *********************/
unsigned char seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 0;

/************************* Timer1 initialization Code *************************/
void TIMER1_Init(void )
{
	TCNT1 = 0; // to make the timer start counting from 0
	OCR1A  = 1000; //compare match value ,, interrupt every 1 second = 1000ms
	TCCR1A = (1<<FOC1A);  // for non PWM set : FOC1A and FOC1B
	TIMSK |= (1<<OCIE1A); // to enable interrupt for compare mode
	//WGM12=1 to enable compare mode ,, cs12 and cs10 for N = 1024
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);
}

/************************** for Timer1 compare mode ***************************/
ISR(TIMER1_COMPA_vect) // called every one second == 1000ms
{
	seconds ++; // increment seconds
	if (seconds == 60)
	{
		seconds = 0;
		minutes ++;
	}
	if (minutes == 60)
	{
		seconds = 0;
		minutes = 0;
		hours ++;
	}
	if (hours == 24)
	{
		seconds = 0;
		minutes = 0;
		hours = 0 ;
	}

}

/************************* INT0 initialization Code *************************/
void INT0_Init(void) //Responsible for reset with falling edge
{
	/* enable external interrupt pin INT0 */
	GICR  |= (1<<INT0);
	/* for edge trigger of INT0 with the falling edge */
	MCUCR |= (1<<ISC01);
}

/** called when push button is pressed due to it is pull up then will cause falling edge **/
ISR(INT0_vect)
{
	seconds = 0;
	minutes = 0;
	hours = 0;

	TCNT1 = 0;
}

/************************* INT1 initialization Code *************************/
void INT1_Init(void) //Responsible for reset with raising edge
{
	/* enable external interrupt pin INT0 */
	GICR  |= (1<<INT1);
	/* for edge trigger of INT1 with the raising edge */
	MCUCR |= (1<<ISC10);
	MCUCR |= (1<<ISC11);
}

/** called when push button is pressed due to it is pull down then will cause raising edge **/
ISR(INT1_vect)
{
    //clear timer clock source bits to disable timer to pause the stop watch
	TCCR1B &= ~(1<<CS10) &~(1<<CS11) &~(1<<CS12);
}

/************************* INT0 initialization Code *************************/
void INT2_Init(void) //Responsible for resume with falling edge
{
	/* enable external interrupt pin INT0 */
	GICR  |= (1<<INT2);
	/* for edge trigger of INT2 with the falling edge */
	MCUCSR &=~(1<<ISC2);

}

/** called when push button is pressed due to it is pull up then will cause falling edge **/
ISR(INT2_vect)
{
	//enable timer clock source bits to enable timer to resume the stop watch
	// NOTE: we enable only CS10 and CS12 for N = 1024
	TCCR1B = (1<<CS10) | (1<<CS12) | (1<<WGM12);
}

int main()
{
	/* make first 6 pins in PORTA as output pins = 1 ,, 0011 1111 ,, for display of 7 segments */
	DDRA|= 0X3F;
	/* make first 4 pins in PORTC output pins = 1 ,, 0000 1111 ,, for decoder */
	DDRC|= 0x0f;
	/* enable all 7 segment ,, 0011 1111 ,, and display on it zero by decoder */
	PORTA |= 0x3F;
	/* initialize output pin for decoder as zero to display it on 7 segment */
	PORTC &= 0xF0;

	/*  clear pin 3 in PORTD to be as input pin = 0 ,, for push button */
	DDRD &= ~(1<<PD2);
	/* set bit ,, activate internal pull up for int0 */
	PORTD |= (1<<PD2);

	/*clear pin 4 in PORTD to be as input pin = 0 ,, for push button */
	DDRD &= ~(1<<PD3);

	/*  clear pin 3 in PORTB to be as input pin = 0 ,, for push button */
	DDRB &= ~(1<<PB2);
	/* set bit ,, activate internal pull up for int2 */
	PORTB |= (1<<PB2);

	/* enable global interrupts I-bit */
	SREG  |= (1<<7);

	// activate external interrupts INT0 , INT1 , INT2
	INT0_Init();
	INT1_Init();
	INT2_Init();

	/* start Timer1 */
	TIMER1_Init();


	while(1)
	{
		/* for enable segment 1 which is responsible for SEC1 */
		PORTA = (PORTA & 0xC0) | 0x01;
		/** NOTE: PORTC & 1111 0000 = 1111 0000 | seconds % 10 as if seconds = 2 then 2 % 10 = 2
		 * if seconds > 10 = 13 as example >> 13 % 10 = 3 will be displayed on SEC1 segment
		 * and then 13 / 10 = 1.3 approximately = 1 will be displayed on SEC2 segment **/
		PORTC = (PORTC & 0XF0) | (seconds % 10);
		/* for notice of change that happen if seconds >= 10 */
		_delay_ms(3);

		/* for enable segment 2 which is responsible for SEC2 */
		PORTA = (PORTA & 0xC0) | 0x02;
		/* PORTC & 1111 0000 = 1111 0000 | seconds / 10 as if seconds = 13 then 13 / 10 = 1 */
		PORTC = (PORTC & 0XF0) | (seconds / 10);
		_delay_ms(3);

		/* for enable segment 3 which is responsible for MIN1 */
		PORTA = (PORTA & 0xC0) | 0x04;
		PORTC = (PORTC & 0XF0) | (minutes % 10);
		_delay_ms(3);

		/* for enable segment 4 which is responsible for MIN2 */
		PORTA = (PORTA & 0xC0) | 0x08;
		PORTC = (PORTC & 0XF0) | (minutes / 10);
		_delay_ms(3);

		/* for enable segment 5 which is responsible for HOUR1 */
		PORTA = (PORTA & 0xC0) | 0x10;
		PORTC = (PORTC & 0XF0) | (hours % 10);
		_delay_ms(3);

		/* for enable segment 6 which is responsible for HOUR2 */
		PORTA = (PORTA & 0xC0) | 0x20;
		PORTC = (PORTC & 0XF0) | (hours / 10);
		_delay_ms(3);

	}
}
