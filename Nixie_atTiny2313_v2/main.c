
#define _INCLUDES_
#define _DEFINES_
#define _VARIABLES_
#define _INTERRUPTS_
#define _MACROS_


//#define _TIME_HARD_SETUP_


#ifdef _INCLUDES_

#include "main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdbool.h>

#include "spi/shift_reg.h"
//#include "spi/usi_spi.h"
#include "ds1307/ds1307.h"
#include "rtos/rtos.h"
//#include "cmd/cmd_interp.h"
//#include "cbuf/cbuf.h"
#include "cbuf/fifo.h"

#endif // _INCLUDES_


#ifdef _DEFINES_

#define DISPLAY_TIMEOUT		5	// In MainTimer cycles
#define DISPLAY_DATE_EVERY	13	// In MainTimer cycles

#define DIGITS_1_PORT	PORTD
#define DIGITS_2_PORT	PORTA
#define DIGITS_3_PORT	PORTA

#define DIGITS_1_DDR	DDRD
#define DIGITS_2_DDR	DDRA
#define DIGITS_3_DDR	DDRA

#define DIGITS_1_PIN	PIND4
#define DIGITS_2_PIN	PINA0
#define DIGITS_3_PIN	PINA1

#endif

#ifdef _MACROS_

#define DIGITS_1_INVERT		DIGITS_1_PORT ^= (1<<DIGITS_1_PIN)
#define DIGITS_2_INVERT		DIGITS_2_PORT ^= (1<<DIGITS_2_PIN)
#define DIGITS_3_INVERT		DIGITS_3_PORT ^= (1<<DIGITS_3_PIN)

#define DIGITS_ALL_ON		DIGITS_1_PORT |= (1<<DIGITS_1_PIN); DIGITS_2_PORT |= (1<<DIGITS_2_PIN); DIGITS_3_PORT |= (1<<DIGITS_3_PORT)

#define DEC2BCD(dec) (((dec / 10) << 4) + (dec % 10))

#endif

#ifdef _VARIABLES_

uint8_t display_bcd[3] = {0,0,0};
volatile bool next_second = false;
uint8_t display_sate = 1; // 0 - time, 1 - date, 2 - custom
uint8_t setup_state = 0; // 0 - normal, 1 - Year, 2 - month, 3 - day, 4 - day of week, 5 - hours, 6 - minutes

uint8_t current_month = 1;

#if defined(CBUF_H)
	cbf_t Rx_buffer;
	cbf_t Tx_buffer;
#elif defined(FIFO_H_)
	FIFO(16) Rx_buffer;
	FIFO(16) Tx_buffer;
#endif

#endif

#ifdef _INTERRUPTS_

ISR(TIMER1_CAPT_vect)
{
	TimerService();
}

ISR(USART_RX_vect)
{
	uint8_t rx_byte = UDR;
	#if defined(CBUF_H)
	cbf_put(&Tx_buffer, rx_byte); // echo
	cbf_put(&Rx_buffer, rx_byte);
	#elif defined(FIFO_H_)
	FIFO_PUT(Tx_buffer, rx_byte);
	FIFO_PUT(Rx_buffer, rx_byte);
	//parse_input_char(rx_byte);
	#endif
	
	if (rx_byte == '\r')
	{
		AddTask(execute_command);
	}
}

ISR(USART_UDRE_vect)
{
	#if defined(CBUF_H)
	if (cbf_isempty(&Tx_buffer))
	{
		UCSRB &= ~(1<<UDRIE);
		} else {
		UDR = cbf_get(&Tx_buffer);
	}
	#elif defined(FIFO_H_)
	if (FIFO_IS_EMPTY(Tx_buffer))
	{
		UCSRB &= ~(1<<UDRIE);
		} else {
		UDR = FIFO_GET(Tx_buffer);
	}
	#endif
	
}

#endif


int main(void)
{
	//wdt_disable();
	init();
	cycle_display_state();
	second_tick_task();
	
	#ifdef _TIME_HARD_SETUP_
	time_first_setup();
	#endif
	
	sei();
	
    while (1) 
    {
		wdt_reset();
		TaskManager();
		
		#if defined(CBUF_H)
			if (!cbf_isempty(&Tx_buffer))	UCSRB |= (1<<UDRIE);
		#elif defined(FIFO_H_)
			if (!FIFO_IS_EMPTY(Tx_buffer))  UCSRB |= (1<<UDRIE);
		#endif	
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


void second_tick_task(void)
{
	
	switch(setup_state)
	// 0 - normal, 1 - Year, 2 - month, 3 - day, 4 - day of week, 5 - hours, 6 - minutes
	{
		case 0:	// normal
			DIGITS_ALL_ON;
			update_display();
			break;
		case 1:	// Year
			DIGITS_1_INVERT;
			break;
		case 5:	// hour
			DIGITS_1_INVERT;
			break;
		case 2:	// month
			DIGITS_2_INVERT;
			break;
		case 6:	// minute
			DIGITS_2_INVERT;
			break;
		case 3:	// day
			DIGITS_3_INVERT;
			break;
		case 4:	// day of week
			DIGITS_3_INVERT;
			break;
		default:
			DIGITS_ALL_ON;
			update_display();
			break;
	}
	
	send_curr_display();
	
	AddTimerTask(second_tick_task, 1);
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
			switch(setup_state)
			{
				case 0:
					ds1307_read(0x03, display_bcd, 3); // [day of week, Day, Month]
					current_month = (((display_bcd[2])>>4)*10) + ((display_bcd[2]) & 0b00001111);
					break;
				case 4: // day of week
					ds1307_read(0x03, display_bcd, 1); // [day of week]
					display_bcd[1] = 0;
					display_bcd[2] = 0;
					break;
				default:
					ds1307_read(0x04, display_bcd, 3); // [Day, Month, Year]
					break;
			}
			break;
			
		default:
			break;
	}
	
	#if defined(SHIFT_REG_H_)
		shift_send_three_reverse_bytes(display_bcd);
	#elif defined(USI_SPI_H_)
		SPI_SS_LOW;
		usi_spi_send_receive(~(display_bcd[0]));
		usi_spi_send_receive(~(display_bcd[1]));
		usi_spi_send_receive(~(display_bcd[2]));
		SPI_SS_HIGH;
	#endif
		
}

void init(void)
{
	#if defined(SHIFT_REG_H_)
		shift_init();
		//shift_send_three_reverse_bytes(display_bcd); // Sends 00:00:00 as fast as it can
	#elif defined(USI_SPI_H_)
		SPI_SS_ST_DDR |= (1<<SPI_SS_ST_PIN);
		usi_spi_master_init();
		SPI_SS_LOW;
		usi_spi_send_receive(0xFF);
		usi_spi_send_receive(0xFF);
		usi_spi_send_receive(0xFF);
		SPI_SS_HIGH;
	#endif
		
	
	cbi(DS_SQW_DDR, DS_SQW_PIN); // configure as input
	cbi(DS_SQW_PORT, DS_SQW_PIN); // disable pullup (need to be enabled if no external pullup)
	
	
	TCCR1A = (0<<WGM11)|(0<<WGM10);
	TCCR1B = (0<<WGM13)|(0<<WGM12)|(0<<ICNC1)|(1<<ICES1); // normal port operation; disable noise canceller on ICP1; rising edge;
	
	//TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10); // clk/1
	
	TIMSK = (1<<ICIE1);	// InputCapture Interrupt Enable
	
	
	ds1307_init();
	TIO_Init();
	
	rtos_init();
	
	#if defined(CBUF_H)
		cbf_init(&Rx_buffer);
		cbf_init(&Tx_buffer);
	#endif

	
}

void pwm_init(void)
{
	//// Timer0 fast PWM, clk/1
	//
	//// Timer1 fast PWM 8-bit, clk/1
	//TCCR1A = (0<<WGM11)|(1<<WGM10);
	//TCCR1B = (0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);
	//
	
	
}

void execute_command(void)
{
	#ifdef _COMMENT_

	uint8_t cmd_args[3];
	uint8_t args_count = 0;
	uint8_t cmd_name[5];
	
	uint8_t i = 0;
	while (i < 4 && !FIFO_IS_EMPTY(Rx_buffer) && (' ' != FIFO_PEEK(Rx_buffer)))
	{
		cmd_name[i] = FIFO_GET(Rx_buffer);
		i++;
	}
	cmd_name[i] = 0;
	
	args_count = FIFO_COUNT(Rx_buffer);
	if (args_count != 7 && args_count != 2) 
	{
		FIFO_FLUSH(Rx_buffer);
		return;
	}
	
	// remove space
	FIFO_GET(Rx_buffer);
	args_count--;
	
		
	
	if (str_equal(cmd_name, "day") && (args_count == 1))
	{
		cmd_args[0] = FIFO_GET(Rx_buffer) - '0';
		ds1307_write(0x03, cmd_args, 1);
	} else
	{
		cmd_args[2] =	((FIFO_GET(Rx_buffer) - '0')<<4);
		cmd_args[2] |=	(FIFO_GET(Rx_buffer) - '0');
		cmd_args[1] =	((FIFO_GET(Rx_buffer) - '0')<<4);
		cmd_args[1] |=	(FIFO_GET(Rx_buffer) - '0');
		cmd_args[0] =	((FIFO_GET(Rx_buffer) - '0')<<4);
		cmd_args[0] |=	(FIFO_GET(Rx_buffer) - '0');
		
		//uint8_t addr;
		if (str_equal(cmd_name, "date"))
		{		
			ds1307_write(0x04, cmd_args, 3);
		} else if (str_equal(cmd_name, "time"))
		{
			ds1307_write(0x00, cmd_args, 3);
		}
		
	}
#endif
	
}


void usart_sendString(uint8_t *string)
{
	uint8_t i = 0;
	while(!FIFO_IS_FULL(Tx_buffer) && string[i] != '\0')
	{
		FIFO_PUT(Tx_buffer, string[i]);
		i++;
	}
	// Try to send immediately
	UCSRB |= (1<<UDRIE);
	
}

#ifdef _COMMENT_
void parse_input_char(uint8_t rx_byte)
{
	static uint8_t cmd_input_state = 0;
	
	switch (cmd_input_state)
	{
		case 0: // 1st byte
			switch(rx_byte)
			{
				case 'd':
					cmd_input_state = 1; // will be date | day
					break;
				case 't':
					cmd_input_state = 51; // will be time
					break;
				default: // space, backspace or something else not character
					if (rx_byte > 0x21)
					{
						cmd_input_state = 231; // unknown, goto help
					}
					break;
			}
			break;
		
		//////////////////////////////////////////////////////////////////////////
		// date
		//////////////////////////////////////////////////////////////////////////
		case 1: // 2st byte date
			switch(rx_byte)
			{
				case 'a': // ok
					cmd_input_state = 2;
					break;
				case 0x08: // backspace
					cmd_input_state = 0;
					break;
				default: 
					cmd_input_state = 232; // unknown, goto help
					break;
			}
			break;	
		case 2: // 3rd byte date | day
			switch (rx_byte)
			{
				case 't': // ok, it's date
					cmd_input_state = 3;
					break;
				case 'y': // ok, it's day
					cmd_input_state = 103;
					break;
				case 0x08: // backspace
					cmd_input_state = 0;
					break;
				default:
					cmd_input_state = 233; // unknown, goto help
					break;
			}
			break;
		case 3:
			switch (rx_byte)
			{
				case 'e': // ok, it's still date
					cmd_input_state = 4;
					break;
				case 0x08: // backspace
					cmd_input_state = 2;
					break;
				default:
					cmd_input_state = 234; // unknown, goto help
					break;
			}
			break;
		case 4:
			switch (rx_byte)
			{
				case ' ': // ok, goto args[0]
					cmd_name[0] = 'd';
					cmd_name[1] = 'a';
					cmd_name[2] = 't';
					cmd_name[3] = 'e';
					cmd_name[4] = 0;
					cmd_input_state = 200; 
					break;
				case 0x08: // backspace
					cmd_input_state = 3;
					break;
				default:
					cmd_input_state = 235; // unknown, goto help
					break;
			}
			break;
		
		
		//////////////////////////////////////////////////////////////////////////
		// day
		//////////////////////////////////////////////////////////////////////////
		case 104:
			case ' ': // ok, goto args[0]
				cmd_name[0] = 'd';
				cmd_name[1] = 'a';
				cmd_name[2] = 'y';
				cmd_name[3] = 0;
				cmd_name[4] = 0;
				cmd_input_state = 200;
				break;
			case 0x08: // backspace
				cmd_input_state = 2;
				break;
			default:
				cmd_input_state = 234; // unknown, goto help
				break;
				
		
		
			
	}
}
#endif


#ifdef _TIME_HARD_SETUP_
void time_first_setup(void)
{
	uint8_t tmp;
	
	tmp = DEC2BCD(47);	// minutes = 40
	ds1307_write(0x01, &tmp, 1);
	
	tmp = DEC2BCD(17);	// hours
	ds1307_write(0x02, &tmp, 1);
	
	tmp = DEC2BCD(7);	// Day Of Week
	ds1307_write(0x03, &tmp, 1);
	
	tmp = DEC2BCD(28);	// Day
	ds1307_write(0x04, &tmp, 1);
	
	tmp = DEC2BCD(2);	// Month
	ds1307_write(0x05, &tmp, 1);
	
	tmp = DEC2BCD(16);	// year
	ds1307_write(0x04, &tmp, 1);
	
	
	
	
}
#endif

void send_curr_display(void)
{
	uint8_t tmp[11];
	
	tmp[0] = ((display_bcd[2])>>4) + '0';			// 10h
	tmp[1] = ((display_bcd[2]) & 0b00001111) + '0';	// h
	tmp[2] = '.';
	tmp[3] = ((display_bcd[1])>>4) + '0';			// 10m
	tmp[4] = ((display_bcd[1]) & 0b00001111) + '0';	// m
	tmp[5] = '.';
	tmp[6] = ((display_bcd[0])>>4) + '0';			// 10s
	tmp[7] = ((display_bcd[0]) & 0b00001111) + '0';	// s
	tmp[8] = '\r';
	tmp[9] = '\n';
	tmp[10] = '\0';
	usart_sendString(tmp);	
	
}