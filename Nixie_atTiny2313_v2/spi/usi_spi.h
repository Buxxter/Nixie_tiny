#ifndef USI_SPI_H_
#define USI_SPI_H_

#include "../main.h"

#define PORT_SPI PORTB
#define DDR_SPI  DDRB
#define PIN_SPI  PINB
#define UCSK     PB7
#define DO       PB6
#define DI       PB5
#define SS       PB4

void usi_spi_master_init (void);
unsigned char usi_spi_send_receive (unsigned char data_w); //функция передачи/приёма байта по USI




#endif /* USI_SPI_H_ */