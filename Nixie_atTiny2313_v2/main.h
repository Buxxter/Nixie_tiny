#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 8000000UL
#include <compat/deprecated.h>
#include "tiny_uart/uart_text_io.h"
//#include "cbuf/cbuf.h"

#define DS_SQW_DDR	DDRD
#define DS_SQW_PORT PORTD
#define DS_SQW_PIN	6

#define SPI_SS_ST_PORT	PORTA
#define SPI_SS_ST_DDR	DDRA
#define SPI_SS_ST_PIN	0

#define SPI_SS_LOW		SPI_SS_ST_PORT &= (~(1<<SPI_SS_ST_PIN))
#define SPI_SS_HIGH		SPI_SS_ST_PORT |= ((1<<SPI_SS_ST_PIN))

#define MAX_CMD_LEN 16



int main(void);
void cycle_display_state(void);
void update_display(void);
void init(void);

void flush_tx(void);

void usart_sendString(char *str);
void execute_command(void);



#endif /* MAIN_H_ */