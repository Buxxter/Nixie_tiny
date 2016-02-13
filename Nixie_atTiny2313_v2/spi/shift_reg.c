
#include "shift_reg.h"
#include <util/delay.h>

#define SHIFT_ST_LOW	SHIFT_ST_PORT &= (~(1<<SHIFT_ST_PIN))
#define SHIFT_ST_HIGH	SHIFT_ST_PORT |= ((1<<SHIFT_ST_PIN))

#define SHIFT_SH_LOW	SHIFT_SH_PORT &= (~(1<<SHIFT_SH_PIN))
#define SHIFT_SH_HIGH	SHIFT_SH_PORT |= ((1<<SHIFT_SH_PIN))

#define SHIFT_DS_LOW	SHIFT_DS_PORT &= (~(1<<SHIFT_DS_PIN))
#define SHIFT_DS_HIGH	SHIFT_DS_PORT |= ((1<<SHIFT_DS_PIN))

void shift_init(void)
{
	SHIFT_DS_DDR |= (1<<SHIFT_DS_PIN);
	SHIFT_ST_DDR |= (1<<SHIFT_ST_PIN);
	SHIFT_SH_DDR |= (1<<SHIFT_SH_PIN);
	
	SHIFT_DS_LOW;
	SHIFT_SH_LOW;
	SHIFT_SH_HIGH;
	shift_sendbyte(0, true, false);
	shift_sendbyte(0, false, false);
	shift_sendbyte(0, false,true);
	
}

void shift_sendbyte(uint8_t byte, bool isFrist, bool isLast)
{
	if (isFrist)
	{
		SHIFT_ST_LOW;
	}
	
	SHIFT_SH_LOW;
	
	for (uint8_t b = 0; b < 8; b++)
	{
		if (byte & (1<<b))
		{
			SHIFT_DS_HIGH;
		} else {
			SHIFT_DS_LOW;
		}
		SHIFT_SH_HIGH;
		//_delay_ms(10);
		SHIFT_SH_LOW;
		//_delay_ms(10);
	}
	
	if (isLast)
	{
		SHIFT_ST_HIGH;
	}
}

void shift_send_three_reverse_bytes(uint8_t *bytes)
{
	for (uint8_t b = 0; b < 3; b++)
	{
		shift_sendbyte(~(bytes[b]), b==0, b == 2);
	}
}