/*
 * shift_reg.h
 *
 * Created: 08.02.2016 2:07:58
 *  Author: My
 */ 


#ifndef SHIFT_REG_H_
#define SHIFT_REG_H_

#include "../main.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#define SHIFT_DS_PORT	PORTA
#define SHIFT_ST_PORT	PORTA
#define SHIFT_SH_PORT	PORTD

#define SHIFT_DS_DDR	DDRA
#define SHIFT_ST_DDR	DDRA
#define SHIFT_SH_DDR	DDRD

#define SHIFT_DS_PIN	1
#define SHIFT_ST_PIN	0
#define SHIFT_SH_PIN	2

void shift_init(void);
void shift_sendbyte(uint8_t byte, bool isFrist, bool isLast);
void shift_send_three_reverse_bytes(uint8_t *bytes);


#endif /* SHIFT_REG_H_ */