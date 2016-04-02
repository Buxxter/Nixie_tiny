
#define _INCLUDES_
#define _DEFINES_
#define _VARIABLES_
#define _INTERRUPTS_
#define _MACROS_



#ifdef _INCLUDES_

#include "main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>


#include "spi/shift_reg.h"
//#include "spi/usi_spi.h"
#include "ds1307/ds1307.h"
#include "ds1307/at24c32.h"
#include "rtos/rtos.h"
//#include "cmd/cmd_interp.h"
//#include "tiny_uart/uart_text_io.h"
//#include "cbuf/cbuf.h"

#endif // _INCLUDES_


#ifdef _DEFINES_

#define DISPLAY_TIMEOUT		5	// In MainTimer cycles
#define DISPLAY_DATE_EVERY	17	// In MainTimer cycles

#define DIGITS_1_PORT	PORTD
#define DIGITS_2_PORT	PORTD
#define DIGITS_3_PORT	PORTD
//
#define DIGITS_1_DDR	DDRD
#define DIGITS_2_DDR	DDRD
#define DIGITS_3_DDR	DDRD
//
#define DIGITS_1_PIN	PIND0
#define DIGITS_2_PIN	PIND4
#define DIGITS_3_PIN	PIND5

#endif

#ifdef _MACROS_

#define DIGITS_1_INVERT		DIGITS_1_PORT ^= (1<<DIGITS_1_PIN)
#define DIGITS_2_INVERT		DIGITS_2_PORT ^= (1<<DIGITS_2_PIN)
#define DIGITS_3_INVERT		DIGITS_3_PORT ^= (1<<DIGITS_3_PIN)

#define DIGITS_1_ON			DIGITS_1_PORT |= (1<<DIGITS_1_PIN)
#define DIGITS_2_ON			DIGITS_2_PORT |= (1<<DIGITS_2_PIN)
#define DIGITS_3_ON			DIGITS_3_PORT |= (1<<DIGITS_3_PIN)

#define DIGITS_1_OFF		DIGITS_1_PORT &= ~(1<<DIGITS_1_PIN)
#define DIGITS_2_OFF		DIGITS_2_PORT &= ~(1<<DIGITS_2_PIN)
#define DIGITS_3_OFF		DIGITS_3_PORT &= ~(1<<DIGITS_3_PIN)


#define DIGITS_ALL_ON		DIGITS_1_ON; DIGITS_2_ON; DIGITS_3_ON
#define DIGITS_ALL_OFF		DIGITS_1_OFF; DIGITS_2_OFF; DIGITS_3_OFF

#define DEC2BCD(dec) (((dec / 10) << 4) + (dec % 10))

#endif

#ifdef _VARIABLES_

uint8_t display_bcd[3] = {0,0,0};
uint8_t backlight[3] = {255,255,255};
volatile bool next_second = false;
uint8_t display_sate = 1; // 0 - time, 1 - date, 2 - custom
uint8_t setup_state = 0; // 0 - normal, 1 - Year, 2 - month, 3 - day, 4 - day of week, 5 - hours, 6 - minutes
uint8_t dim_digits = 0;

//uint8_t current_month = 1;

#if defined(CBUF_H)
	cbf_t Rx_buffer;
	cbf_t Tx_buffer;
#elif defined(FIFO_H_)
	FIFO(16) Rx_buffer;
	FIFO(16) Tx_buffer;
#endif

#endif

#ifndef _DS1307_
#include <avr/pgmspace.h>

const uint8_t backlight_consts[] PROGMEM = { \
	0,0,30,		\
	0,10,30,	\
	0,30,10,	\
	0,30,0,		\
	10,30,0,	\
	30,10,0,	\
	30,0,0,		\
	30,10,0,	\
	30,30,0,	\
	30,10,0,	\
	30,0,10,	\
	0,5,30		\
};

#endif

#ifdef _INTERRUPTS_

ISR(TIMER1_CAPT_vect)
{
	TimerService();
}

#ifdef _UART_TIO_H_

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

#endif

#ifdef _UART_TIO_H_

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
	#ifdef _BACKLIGHT_SETUP_
	backlight_first_setup();
	#endif
	
	sei();
	
    while (1) 
    {
		wdt_reset();
		TaskManager();
		
		#ifdef _UART_TIO_H_

			#if defined(CBUF_H)
				if (!cbf_isempty(&Tx_buffer))	UCSRB |= (1<<UDRIE);
			#elif defined(FIFO_H_)
				if (!FIFO_IS_EMPTY(Tx_buffer))  UCSRB |= (1<<UDRIE);
			#endif	
		
		#endif // _UART_TIO_H_
		
		digits_dimmer();
		
    }
}

void digits_dimmer(void)
{
	static uint8_t dim_counter;
	#define DIM_CUT 3
	
	uint8_t del1 = 1;
	uint8_t del2 = 1;
	//if (dim_digits == 0) 
	//{
		////DIGITS_ALL_ON;
		//
		//DIGITS_ALL_OFF;
		//DIGITS_1_ON;
		//_delay_ms(del1);
		//DIGITS_1_OFF;
		//_delay_ms(del2);
		//DIGITS_2_ON;
		//_delay_ms(del1);
		//DIGITS_2_OFF;
		//_delay_ms(del2);
		//DIGITS_3_ON;
		//_delay_ms(del1);
		//DIGITS_3_OFF;
		//_delay_ms(del2);
		//return;
	//}
	
	DIGITS_ALL_OFF;	
	if ((dim_counter < DIM_CUT) || !((1<<2) & dim_digits))
	{
		DIGITS_1_ON;
		_delay_ms(del1);
		DIGITS_1_OFF;
	}
	_delay_ms(del2);
	
	if ((dim_counter < DIM_CUT) || !((1<<1) & dim_digits))
	{
		DIGITS_2_ON;
		_delay_ms(del1);
		DIGITS_2_OFF;
	}
	_delay_ms(del2);
	
	if ((dim_counter < DIM_CUT) || !((1<<0) & dim_digits))
	{
		DIGITS_3_ON;
		_delay_ms(del1);
		DIGITS_3_OFF;
	}
	_delay_ms(del2);
	
	dim_counter++;
	if (dim_counter > (DIM_CUT + DIM_CUT)) dim_counter = 0;
	
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
			//DIGITS_ALL_ON;
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
			//DIGITS_ALL_ON;
			update_display();
			break;
	}
	
	//send_curr_display();
	
	AddTimerTask(second_tick_task, 1);
}

void update_display(void)
{
	uint8_t current_month_addr;
	uint8_t tmp;
	switch(display_sate)
	{
		case 0:
			ds1307_read(0x00, display_bcd, 3);	// [Seconds, Minutes, Hours]
			display_bcd[2] &= ~(1<<6); // Remove 12/24 from Hours
			display_bcd[0] &= ~(1<<7); // Remove CH from Seconds
			
			dim_digits = 0;			
			break;
		case 1:
			switch(setup_state)
			{
				case 0:
					ds1307_read(0x03, display_bcd, 3); // [day of week, Day, Month]
					
					current_month_addr = ((display_bcd[2] & 0b00001111) - 1) * 3;
					tmp = display_bcd[2];
					display_bcd[2] = display_bcd[1];
					display_bcd[1] = tmp;
					#if defined(_DS3231_)
						
						display_bcd[2] = display_bcd[2] & (~(1<<7));
						
						backlight[0] = pgm_read_byte(&(backlight_consts[current_month_addr]));
						backlight[1] = pgm_read_byte(&(backlight_consts[current_month_addr + 1]));
						backlight[2] = pgm_read_byte(&(backlight_consts[current_month_addr + 2]));
					
						//at24c32_read(0, (current_month_addr) * 3, &backlight[0], 1);
						//at24c32_read(0, ((current_month_addr) * 3) + 1, &backlight[1], 1);
						//at24c32_read(0, ((current_month_addr) * 3) + 2, &backlight[2], 1);
					
					#elif defined(_DS1307_)
						ds1307_read((0x08) + current_month_addr, backlight, 3);
					#endif
					
					dim_digits |= (1<<0);
					
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
	update_backlight();
	
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
	
	DIGITS_1_DDR |= (1<<DIGITS_1_PIN);
	DIGITS_2_DDR |= (1<<DIGITS_2_PIN);
	DIGITS_3_DDR |= (1<<DIGITS_3_PIN);
		
	
	cbi(DS_SQW_DDR, DS_SQW_PIN); // configure as input
	cbi(DS_SQW_PORT, DS_SQW_PIN); // disable pullup (need to be enabled if no external pullup)
	
	
	hardware_timers_init();	
	ds1307_init();
	
	#ifdef _UART_TIO_H_
	TIO_Init();
	#endif // _UART_TIO_H_
	
	
	rtos_init();
	
	#if defined(CBUF_H)
		cbf_init(&Rx_buffer);
		cbf_init(&Tx_buffer);
	#endif

	
}

void hardware_timers_init(void)
{
	
	
	//TCCR1A = (0<<WGM11)|(0<<WGM10);
	TCCR1B = (0<<WGM13)|(0<<WGM12)|(0<<ICNC1)|(1<<ICES1); // normal port operation; disable noise canceller on ICP1; rising edge;
	
	//TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10); // clk/1
	
	TIMSK = (1<<ICIE1);	// InputCapture Interrupt Enable
	
	// =========
	// Timer1 fast PWM 8-bit, clk/1
	TCCR1A = (0<<WGM11)|(1<<WGM10); // fastPWM 8-bit
	TCCR1B |= (0<<WGM13)|(1<<WGM12); // fastPWM 8-bit
	
	#if defined(_BACKLIGHT_AND_)
		TCCR1A |= (1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0);	// Clear OC1A on CompareMatch, set on TOP
																	// Clear OC1B on CompareMatch, set on TOP
	#elif defined(_BACKLIGHT_KAT_)
		TCCR1A |= (1<<COM1A1)|(1<<COM1A0)|(1<<COM1B1)|(1<<COM1B0);	// Set OC1A on CompareMatch, clear on TOP
																	// Set OC1B on CompareMatch, clear on TOP
	#endif
	
	TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10); // clk/1
	
	
	// Timer0 fast PWM, clk/1
	#if defined(_BACKLIGHT_AND_)
		TCCR0A  = (1<<COM0A1)|(0<<COM0A0);		// Clear OC0A on CompareMatch, set on TOP
	#elif defined(_BACKLIGHT_KAT_)
		TCCR0A  = (1<<COM0A1)|(1<<COM0A0);		// Set OC0A on CompareMatch, clear on TOP
	#endif
	
	TCCR0A &= ~((1<<COM0B1)|(1<<COM0B0));	// OC0B normal port operation
	TCCR0A |= (1<<WGM01)|(1<<WGM00);		// fastPWM
	
	TCCR0B = (0<<CS02)|(0<<CS01)|(1<<CS00);	// clk/1	
	
	DDRB |= (1<<PINB4)|(1<<PINB3)|(1<<PINB2);
		
}

void update_backlight(void)			// (uint8_t red, uint8_t green, uint8_t blue)
{
	OCR0A = backlight[0]; // red;
	OCR1A = backlight[1]; // green;
	OCR1B =	backlight[2]; // blue;
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

#ifdef _UART_TIO_H_
void usart_sendString(uint8_t *string)
{
	#if defined(FIFO_H_)
	
	uint8_t i = 0;
	while(!FIFO_IS_FULL(Tx_buffer) && string[i] != '\0')
	{
		FIFO_PUT(Tx_buffer, string[i]);
		i++;
	}
	// Try to send immediately
	UCSRB |= (1<<UDRIE);
	
	#elif defined(CBUF_H)
	
	#endif
	
}
#endif

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
	
	//tmp = DEC2BCD(0);	// seconds = 0
	//ds1307_write(0x00, &tmp, 1);
	//
	//tmp = DEC2BCD(10);	// minutes = 40
	//ds1307_write(0x01, &tmp, 1);
	//
	//tmp = DEC2BCD(4);	// hours
	//ds1307_write(0x02, &tmp, 1);
	//
	//tmp = DEC2BCD(2);	// Day Of Week
	//ds1307_write(0x03, &tmp, 1);
	
	tmp = DEC2BCD(10);	// Day
	ds1307_write(0x04, &tmp, 1);
	
	tmp = DEC2BCD(3);	// Month
	ds1307_write(0x05, &tmp, 1);
	
	tmp = DEC2BCD(16);	// year
	ds1307_write(0x06, &tmp, 1);
	
}
#endif

#ifdef _BACKLIGHT_SETUP_
void backlight_first_setup(void)
{
	uint8_t tmp[36] = { \
						0,0,50,	\
						0,20,50,	\
						0,50,20,	\
						0,50,0,	\
						20,50,0,	\
						50,20,0,	\
						50,0,0,	\
						50,20,0,	\
						50,50,0,	\
						50,20,0,	\
						50,0,20,	\
						0,5,50	\
		};
	
	at24c32_write(0, 3, tmp, 36);
	
	
	
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