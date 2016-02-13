
#include "ds1307.h"
#include "../main.h"
#include <util/delay.h>

bool ds1307_init(void)
{
	i2c_init();
	
	uint8_t tmp;
	// set 24H mode
	if (!(ds1307_read(0x02, &tmp))) return false;
	
	
	if (bit_is_set(tmp, 6)) // 12H mode, must reset hours value
	{
		if (!(ds1307_write(0x02, tmp & (~(1<<6))))) return false;
	}
	
	// set ClockHalt (CH) to 0
	if (!(ds1307_read(0x00, &tmp))) return false;
	tmp &= ~(1<<7);	
	if (!(ds1307_write(0x00, tmp))) return false;
		
	// Enable SQWE at [see below]
	if (!(ds1307_write(0x07, 0b00010000))) return false; 
	
	//Clock_SetUserData(0x07, 0b00000000); // disabled
	//Clock_SetUserData(0x07, 0b10010000); // 1Hz
	//Clock_SetUserData(0x07, 0b10010001); // 4.096kHz
	//Clock_SetUserData(0x07, 0b10010010); // 8.192kHz
	//Clock_SetUserData(0x07, 0b10010011); // 32.768kHz
	
	
	return true;
}

bool ds1307_read(uint8_t address, uint8_t *data_address)
{
	
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// Старт (запись)
	if(i2c_write(address) != 0)	return false;					// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
	i2c_stop();													// Стоп (для последующего чтения)
	
	i2c_start_wait(DS1307_ADDR + I2C_READ);						// Старт (чтение)
	*data_address = i2c_readNak();								// Чтение с завершением из регистра, на котором стоит внутренний счетчик
	
	i2c_stop();													// Стоп
	return true;	
}

bool ds1307_write(uint8_t address, uint8_t data)
{
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// Старт (запись)
	if(i2c_write(address) != 0)					return false;	// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
	if(i2c_write(data) != 0)					return false;	// Запись данных в регистр, на котором стоит внутренний счетчик
	i2c_stop();													// Стоп
	return true;
}
