

#include "main.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "spi/shift_reg.h"
#include "ds1307/ds1307.h"

uint8_t bytes[3] = {0,0,0};

int main(void)
{
	shift_init();
	shift_send_three_reverse_bytes(bytes); // Sends 00:00:00 as fast as it can
	TIO_Init();
		    
	ds1307_init();	
	
    while (1) 
    {
				
		ds1307_read(0x02, &bytes[2]);
		ds1307_read(0x01, &bytes[1]);
		ds1307_read(0x00, &bytes[0]);
		
		shift_send_three_reverse_bytes(bytes);
		_delay_ms(100);
		
		
    }
}

