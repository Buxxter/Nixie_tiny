
#include "at24c32.h"
#include "../main.h"
#include <util/delay.h>

bool at24c32_init(void)
{
	i2c_init();
	return true;
}

bool at24c32_read(uint8_t address_h, uint8_t address_l, uint8_t *data, uint8_t bytes)
{
	uint8_t i = 0;
	if(i2c_start(AT24C32_ADDR + I2C_WRITE) != 0)	return false;	// Старт (запись)
	if(i2c_write(address_h) != 0)	return false;					// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
	if(i2c_write(address_l) != 0)	return false;					// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
	//i2c_stop();													// Стоп (для последующего чтения)
	
	i2c_start(AT24C32_ADDR + I2C_READ);							// Старт (чтение) (был i2c_start_wait, пробуем сэкономить место)
	while(i < bytes - 1)
	{
		data[i] = i2c_readAck();
		i++;
	}
	data[i] = i2c_readNak();									// Чтение с завершением из регистра, на котором стоит внутренний счетчик
	//i2c_stop();												// Стоп (i2c_readNak()  уже делает стоп)
	
	return true;
}

bool at24c32_write(uint8_t address_h, uint8_t address_l, uint8_t *data, uint8_t bytes)
{
	uint8_t i = 0;
	if(i2c_start(AT24C32_ADDR + I2C_WRITE) != 0)	return false;	// Старт (запись)
	if(i2c_write(address_h) != 0)						return false;	// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
	if(i2c_write(address_l) != 0)						return false;	// Запись адреса регистра, чтобы сдвинуть внутренний счетчик на нужный адрес
		
	while(i <= bytes - 1)
	{
		if(i2c_write(data[i]) != 0)					return false;	// Запись данных в регистр, на котором стоит внутренний счетчик
		i++;
	}
	i2c_stop();													// Стоп
	return true;
}
