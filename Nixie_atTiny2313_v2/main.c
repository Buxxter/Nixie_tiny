

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
#include "cmd/cmd_interp.h"
//#include "cbuf/cbuf.h"
#include "cbuf/fifo.h"

#define DISPLAY_TIMEOUT		5	// In MainTimer cycles
#define DISPLAY_DATE_EVERY	13	// In MainTimer cycles

uint8_t display_bcd[3] = {0,0,0};
volatile bool next_second = false;

#if defined(CBUF_H)
	cbf_t Rx_buffer;
	cbf_t Tx_buffer;
#elif defined(FIFO_H_)
	FIFO(16) Rx_buffer;
	FIFO(16) Tx_buffer;
#endif


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
	
	#if defined(SHIFT_REG_H_)
		shift_send_three_reverse_bytes(display_bcd);
	#elif defined(USI_SPI_H_)
		SPI_SS_LOW;
		usi_spi_send_receive(~(display_bcd[0]));
		usi_spi_send_receive(~(display_bcd[1]));
		usi_spi_send_receive(~(display_bcd[2]));
		SPI_SS_HIGH;
	#endif
		
	
	AddTimerTask(update_display, 1);
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
	#endif
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

void execute_command(void)
{
	//TIO_TextOutput("\r\n");
	//cmd_exec(rx_buffer);
	//rx_index = 0;
	//TIO_TextOutput("\r\n");
}

