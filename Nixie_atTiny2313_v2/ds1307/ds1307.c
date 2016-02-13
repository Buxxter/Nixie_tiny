
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
	
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// ����� (������)
	if(i2c_write(address) != 0)	return false;					// ������ ������ ��������, ����� �������� ���������� ������� �� ������ �����
	i2c_stop();													// ���� (��� ������������ ������)
	
	i2c_start_wait(DS1307_ADDR + I2C_READ);						// ����� (������)
	*data_address = i2c_readNak();								// ������ � ����������� �� ��������, �� ������� ����� ���������� �������
	
	i2c_stop();													// ����
	return true;	
}

bool ds1307_write(uint8_t address, uint8_t data)
{
	if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// ����� (������)
	if(i2c_write(address) != 0)					return false;	// ������ ������ ��������, ����� �������� ���������� ������� �� ������ �����
	if(i2c_write(data) != 0)					return false;	// ������ ������ � �������, �� ������� ����� ���������� �������
	i2c_stop();													// ����
	return true;
}
