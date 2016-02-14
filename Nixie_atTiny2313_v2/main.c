

#include "main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdbool.h>

#include "spi/shift_reg.h"
#include "ds1307/ds1307.h"
#include "rtos/rtos.h"
#include "cmd/cmd_interp.h"

#define DISPLAY_TIMEOUT		5	// In MainTimer cycles
#define DISPLAY_DATE_EVERY	13	// In MainTimer cycles

uint8_t display_bcd[3] = {0,0,0};
volatile bool next_second = false;


uint8_t rx_index = 0;
uint8_t rx_buffer[MAX_CMD_LEN + 1] = "";

uint8_t display_sate = 1; // 0 - time, 1 - date, 2 - custom

int main(void)
{
	//wdt_disable();
	init();
	cycle_display_state();
	update_display();
	
	sei();
	
    while (1) 
    {
		wdt_reset();
		TaskManager();
		//uart_recieve_byte();
    }
}

void cycle_display_state(void)
{
	switch (display_sate)
	{
		case 0:		
			display_sate = 1;
			AddTimerTask(cycle_display_state, DISPLAY_TIMEOUT);
			break;
		default:	
			display_sate = 0;
			AddTimerTask(cycle_display_state, DISPLAY_DATE_EVERY);
			break;
	}
}

void update_display(void)
{
	switch(display_sate)
	{
		case 0:
			ds1307_read(0x00, display_bcd, 3);	// [Seconds, Minutes, Hours]
			display_bcd[2] &= ~(1<<6); // Remove 12/24 from Hours
			display_bcd[0] &= ~(1<<7); // Remove CH from Seconds
			break;
		case 1:
			ds1307_read(0x03, display_bcd, 3); // [day of week, Day, Month]
			break;
		default:
			break;
	}
	
	
	shift_send_three_reverse_bytes(display_bcd);
	
	AddTimerTask(update_display, 1);
}

void init(void)
{
	shift_init();
	shift_send_three_reverse_bytes(display_bcd); // Sends 00:00:00 as fast as it can
	
	cbi(DS_SQW_DDR, DS_SQW_PIN); // configure as input
	cbi(DS_SQW_PORT, DS_SQW_PIN); // disable pullup (need to be enabled if no external pullup)
	
	
	TCCR1A = (0<<WGM11)|(0<<WGM10);
	TCCR1B = (0<<WGM13)|(0<<WGM12)|(0<<ICNC1)|(1<<ICES1); // normal port operation; disable noise canceller on ICP1; rising edge;
	
	//TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10); // clk/1
	
	TIMSK = (1<<ICIE1);	// InputCapture Interrupt Enable
	
	
	ds1307_init();
	TIO_Init();
	
	rtos_init();
	
}

ISR(TIMER1_CAPT_vect)
{
	TimerService();
}

void send_echo(void)
{
	TIO_CharOutput(rx_buffer[rx_index - 1]);
}

void execute_command(void)
{
	TIO_TextOutput("\r\n");
	cmd_exec(rx_buffer);
	rx_index = 0;
	TIO_TextOutput("\r\n");
}

void uart_recieve_byte(void)
{
	if (!(UCSRA & (1<<RXC)))
	{
		return;
	}
	unsigned char rxbyte = UDR;
	
	if (rxbyte == '\r' || rx_index >= MAX_CMD_LEN)
	{
		rx_buffer[rx_index] = 0;
		rx_index = 0;
		AddTask(execute_command);
	} else if (rxbyte != '\n')
	{
		rx_buffer[rx_index] = rxbyte;
		rx_index++;
		send_echo();
	}
	
	
}