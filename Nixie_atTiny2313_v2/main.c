

#include "main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "spi/shift_reg.h"
#include "ds1307/ds1307.h"

uint8_t bytes[3] = {0,0,0};
volatile bool next_second = false;

int main(void)
{
	init();	
	
	sei();
	
    while (1) 
    {
		if (next_second)
		{
			ds1307_read(0x02, &bytes[2]);
			ds1307_read(0x01, &bytes[1]);
			ds1307_read(0x00, &bytes[0]);
		
			shift_send_three_reverse_bytes(bytes);
			next_second = false;
		}
		//_delay_ms(100);
    }
}

void init(void)
{
	shift_init();
	shift_send_three_reverse_bytes(bytes); // Sends 00:00:00 as fast as it can
	
	cbi(DS_SQW_DDR, DS_SQW_PIN); // configure as input
	cbi(DS_SQW_PORT, DS_SQW_PIN); // disable pullup (need to be enabled if no external pullup)
	
	
	TCCR1A = (0<<WGM11)|(0<<WGM10);
	TCCR1B = (0<<WGM13)|(0<<WGM12)|(0<<ICNC1)|(1<<ICES1); // normal port operation; disable noise canceller on ICP1; rising edge;
	
	//TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10); // clk/1
	
	TIMSK = (1<<ICIE1);	// InputCapture Interrupt Enable
	
	
	ds1307_init();
	TIO_Init();
	
	
	
}

ISR(TIMER1_CAPT_vect)
{
	next_second = true;
}