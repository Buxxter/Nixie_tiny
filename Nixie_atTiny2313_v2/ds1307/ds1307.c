
#include "ds1307.h"
#include "../main.h"
#include <util/delay.h>

bool ds1307_init(void)
{
	i2c_init();
	
	uint8_t tmp;
	// set 24H mode
	if (!(ds1307_read(0x02, &tmp, 1))) return false;
		
	// 12H mode, must reset hours value
	tmp &= ~(1<<6);
	if (!(ds1307_write(0x02, &tmp, 1))) return false;
	
	#if defined(_DS1307_)
		
		// set ClockHalt (CH) to 0
		if (!(ds1307_read(0x00, &tmp, 1))) return false;
		tmp &= ~(1<<7);
		if (!(ds1307_write(0x00, &tmp, 1))) return false;
	
		// Enable SQWE at [see below]
		tmp = 0b00010000;
		if (!(ds1307_write(0x07, &tmp, 1))) return false;
	
		//Clock_SetUserData(0x07, 0b00000000); // disabled
		//Clock_SetUserData(0x07, 0b10010000); // 1Hz
		//Clock_SetUserData(0x07, 0b10010001); // 4.096kHz
		//Clock_SetUserData(0x07, 0b10010010); // 8.192kHz
		//Clock_SetUserData(0x07, 0b10010011); // 32.768kHz
		
	#elif defined(_DS3231_)
	
		tmp = 0b00000000; //1Hz, no alarms
		if (!(ds1307_write(0x0E, &tmp, 1))) return false;
		
	#endif // _DS1307
	
	
	
	
	return true;
}

bool ds1307_read(uint8_t address, uint8_t *data, uint8_t bytes)
{
	uint8_t i = 0;
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// Старт (запись)
	if(i2c_write(address) != 0)	return false;					// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
	i2c_stop();													// Стоп (для последующего чтения)
	
	i2c_start(DS1307_ADDR + I2C_READ);							// Старт (чтение) (был i2c_start_wait, пробуем сэкономить место)
	while(i < bytes - 1)
	{
		data[i] = i2c_readAck();
		i++;
	}
	data[i] = i2c_readNak();									// Чтение с завершением из регистра, на котором стоит внутренний счетчик
	//i2c_stop();												// Стоп (i2c_readNak()  уже делает стоп)
	
	return true;	
}

bool ds1307_write(uint8_t address, uint8_t *data, uint8_t bytes)
{
	uint8_t i = 0;
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// Старт (запись)
	if(i2c_write(address) != 0)					return false;	// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
	while(i <= bytes - 1)
	{
		if(i2c_write(data[i]) != 0)				return false;	// Запись данных в регистр, на котором стоит внутренний счетчик
		i++;
	}
	i2c_stop();													// Стоп
	return true;
}
