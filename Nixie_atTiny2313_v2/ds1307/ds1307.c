
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
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// ����� (������)
	if(i2c_write(address) != 0)	return false;					// ������ ������ ��������, ����� �������� ���������� ������� �� ������ �����
	i2c_stop();													// ���� (��� ������������ ������)
	
	i2c_start(DS1307_ADDR + I2C_READ);							// ����� (������) (��� i2c_start_wait, ������� ���������� �����)
	while(i < bytes - 1)
	{
		data[i] = i2c_readAck();
		i++;
	}
	data[i] = i2c_readNak();									// ������ � ����������� �� ��������, �� ������� ����� ���������� �������
	//i2c_stop();												// ���� (i2c_readNak()  ��� ������ ����)
	
	return true;	
}

bool ds1307_write(uint8_t address, uint8_t *data, uint8_t bytes)
{
	uint8_t i = 0;
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// ����� (������)
	if(i2c_write(address) != 0)					return false;	// ������ ������ ��������, ����� �������� ���������� ������� �� ������ �����
	while(i <= bytes - 1)
	{
		if(i2c_write(data[i]) != 0)				return false;	// ������ ������ � �������, �� ������� ����� ���������� �������
		i++;
	}
	i2c_stop();													// ����
	return true;
}
