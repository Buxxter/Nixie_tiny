
#include "shift_reg.h"
//#include <util/delay.h>

#define SHIFT_ST_SS_LOW		SHIFT_ST_SS_PORT &= (~(1<<SHIFT_ST_SS_PIN))
#define SHIFT_ST_SS_HIGH	SHIFT_ST_SS_PORT |= ((1<<SHIFT_ST_SS_PIN))

#define SHIFT_SH_SC_LOW		SHIFT_SH_SC_PORT &= (~(1<<SHIFT_SH_SC_PIN))
#define SHIFT_SH_SC_HIGH	SHIFT_SH_SC_PORT |= ((1<<SHIFT_SH_SC_PIN))

#define SHIFT_DS_MO_LOW		SHIFT_DS_MO_PORT &= (~(1<<SHIFT_DS_MO_PIN))
#define SHIFT_DS_MO_HIGH	SHIFT_DS_MO_PORT |= ((1<<SHIFT_DS_MO_PIN))

void shift_init(void)
{
	SHIFT_DS_MO_DDR |= (1<<SHIFT_DS_MO_PIN);
	SHIFT_ST_SS_DDR |= (1<<SHIFT_ST_SS_PIN);
	SHIFT_SH_SC_DDR |= (1<<SHIFT_SH_SC_PIN);
	
	SHIFT_DS_MO_LOW;
	SHIFT_SH_SC_LOW;
	SHIFT_SH_SC_HIGH;
	shift_sendbyte(0xFF, true, false);
	shift_sendbyte(0xFF, false, false);
	shift_sendbyte(0xFF, false, true);
	
}

void shift_sendbyte(uint8_t byte, bool isFrist, bool isLast)
{
	if (isFrist)
	{
		SHIFT_ST_SS_LOW;
	}
	
	SHIFT_SH_SC_LOW;
	
	for (uint8_t b = 0; b < 8; b++)
	{
		if (byte & (1<<b))
		{
			SHIFT_DS_MO_HIGH;
		} else {
			SHIFT_DS_MO_LOW;
		}
		SHIFT_SH_SC_HIGH;
		//_delay_ms(10);
		SHIFT_SH_SC_LOW;
		//_delay_ms(10);
	}
	
	if (isLast)
	{
		SHIFT_ST_SS_HIGH;
	}
}

void shift_send_three_reverse_bytes(uint8_t *bytes)
{
	#if defined(_SHIFT_REVERSE_)
		for (uint8_t b = 0; b < 3; b++)
		{
			shift_sendbyte((bytes[b]), b==0, b == 2);
		}
	#else
		for (uint8_t b = 3; b > 0; b--)
		{
			shift_sendbyte((bytes[b-1]), b == 3, b == 1);
		}
	#endif
}