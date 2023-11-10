/*
 * project2.c
 *
 *  Created on: Sep 16, 2023
 *      Author: Fatma Albaz
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
/*
 * Global variables used for counting time..
 */
unsigned char Global_Seconds_One =  0;
unsigned char Global_Seconds_Two =  0;
unsigned char Global_Minutes_One =  0;
unsigned char Global_Minutes_Two =  0;
unsigned char Global_Hours_One   =  0;
unsigned char Global_Hours_Two   =  0;

/*
 * Function definitions
 */
void TIMER1_CMP_init (void){// timer 1 on CTC mode
	TCCR1A = (1<<FOC1A);    // Force Output Compare for Compare unit A
	TCCR1B= (1<<WGM12); 	// compare mode selected (mode 4)..
	TCCR1B |= (1<<CS10)| (1<<CS11);	//prescaler: clkI/O/64 (From prescaler)
	TCNT1 = 0;				// initial value is 0
	OCR1A =  15625;			// value of compare match at compare match A
	TIMSK |= (1<<OCIE1A);   // Timer1 output compare A match interrupt enable (interrupt is with A compare match)
}

void INT0_init (void){
	MCUCR |= (1<<ISC01);	// falling edge in interrupt 0
	GICR |= (1<< INT0); 	// external interrupt 0 enable
	DDRD &=~ (1<< PD2);		// pin PD2 as input responsible on INT0
	PORTD|= (1<<PD2);		//internal pull up

}

void INT1_init (void){
	MCUCR |= (1<<ISC11)|(1<<ISC10); 	// rising edge in interrupt 1
	GICR |= (1<< INT1); 				// external interrupt 1 enable
	DDRD &=~  (1<< PD3); 				// pin PD3 as input responsible on INT1
}
void INT2_init (void){
	MCUCSR &=~ (1<<ISC2); 				// falling edge in interrupt 2
	GICR |= (1<< INT2); 				// external interrupt 1 enable
	DDRB &=~  (1<< PB2); 				// pin PB2 as input responsible on INT2
	PORTB|= (1<<PB2);					//internal pull up
}

/*
 * Separate function to display all the values..
 */
void Display_On_7seg (void){
	PORTA &=~ 0x3f;			// disable all of them
	PORTA |= 0b00000001; 	//enable the first one
	PORTC = (PORTC &0xf0)| (Global_Seconds_One &0x0f);
	_delay_ms (2);

	PORTA &=~ 0x3f;			// disable all of them
	PORTA |= 0b00000010;	//enable the second one
	PORTC = (PORTC &0xf0)| (Global_Seconds_Two &0x0f);
	_delay_ms (2);

	PORTA &=~ 0x3f;			// disable all of them
	PORTA |= 0b00000100; 	//enable the third one
	PORTC = (PORTC &0xf0)| (Global_Minutes_One &0x0f);
	_delay_ms (10);

	PORTA &=~ 0x3f;			// disable all of them
	PORTA |= 0b00001000; 	//enable the forth one
	PORTC = (PORTC &0xf0)| (Global_Minutes_Two &0x0f);
	_delay_ms (2);

	PORTA &=~ 0x3f;			// disable all of them
	PORTA |= 0b00010000; 	//enable the fifth one
	PORTC = (PORTC &0xf0)| (Global_Hours_One & 0x0f);
	_delay_ms (2);

	PORTA &=~ 0x3f;			// disable all of them
	PORTA |= 0b00100000; 	//enable the sixth one
	PORTC = (PORTC &0xf0)| (Global_Hours_Two &0x0f);
	_delay_ms (2);
}

/*
 * Main function..
 */
int main (void){
	/*
	 * Global interrupt enable (I-BIT)..
	 */
	SREG |= (1<<7);
	/*
	 * First four pins in port c are output
	 * we write the required number to them
	 */
	DDRC |= 0x0f;
	PORTC &= 0xf0;
	/*
	 * 7 segments enabled
	 */
	DDRA  |= 0x3f; 			// (first 6 pins are output)
	PORTA |= 0x3f; 			// (first 6 pins are LOW "ON common anode")
	DDRD = (1<< PC5);       // red led output for indicating reset
	PORTD &=~ (1<< PC5); 	// red led is off at the beginning

	DDRD = (1<< PC6);       // green led output for indicating resume
	PORTD &=~ (1<< PC6); 	// green led is off at the beginning

	DDRD = (1<< PC7);       // blue led output for indicating pause
	PORTD &=~ (1<< PC7); 	// blue led is off at the beginning

	//calling init functions
	TIMER1_CMP_init ();
	INT0_init();
	INT1_init();
	INT2_init();


	while (1){
		Display_On_7seg();
	}
	return 0;
}



void Increment_values (void){
	if (Global_Seconds_One != 9){
		Global_Seconds_One++;
	}
	else {
		Global_Seconds_One =0;
		if (Global_Seconds_Two !=5){
			Global_Seconds_Two++;
		}
		else {
			Global_Seconds_Two=0;
			if (Global_Minutes_One !=9){
				Global_Minutes_One++;
			}
			else {
				Global_Minutes_One=0;
				if (Global_Minutes_Two !=5){
					Global_Minutes_Two ++;
				}
				else {
					Global_Minutes_Two=0;
					if (Global_Hours_One!=9){
						Global_Hours_One++;
					}
					else {
						Global_Hours_One =0;
						if (Global_Hours_Two !=9){
							Global_Hours_Two ++;
						}
						else {
							Global_Seconds_One =  0;
							Global_Seconds_Two =  0;
							Global_Minutes_One =  0;
							Global_Minutes_Two =  0;
							Global_Hours_One   =  0;
							Global_Hours_Two   =  0;
							PORTD |= (1<< PC5); // RED led on indicating a reset due to overflow
						}
					}

				}
			}
		}

	}

}

ISR (TIMER1_COMPA_vect){
	Increment_values();
}


ISR (INT0_vect){
	TIMER1_CMP_init();
	Global_Seconds_One =  0;
	Global_Seconds_Two =  0;
	Global_Minutes_One =  0;
	Global_Minutes_Two =  0;
	Global_Hours_One   =  0;
	Global_Hours_Two   =  0;
	// indication for a reset event open a red led till we press of pause or resume ..
	PORTD |= (1<< PD5);
	PORTD &=~ (1<< PC6);
	PORTD &=~ (1<< PC7);
}
ISR (INT1_vect){
	// disable clock..
	TCCR1B =0;
	// indication for a Pause event open a red blue till we press of reset or resume ..
	PORTD |= (1<< PC7);
	PORTD &=~ (1<< PD5);
	PORTD &=~ (1<< PC6);
}
ISR (INT2_vect){
	// enable clock at prescaler = 64..
	TIMER1_CMP_init();
	// indication for a Resume event open a green led till we press of pause or reset ..
	PORTD |=  (1<< PC6);
	PORTD &=~ (1<< PD5);
	PORTD &=~ (1<< PC7);
}
