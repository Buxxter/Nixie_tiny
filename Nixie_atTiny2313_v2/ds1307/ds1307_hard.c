
#include "ds1307_hard.h"
#include "../main.h"
//#include <util/delay.h>
#include "usitwim.h"

bool ds1307_init(void)
{
	usitwim_init();
	
	uint8_t tmp;
	uint8_t address;
	
	// set 24H mode
	address = 0x02;
	if (!(ds1307_read(&address, &tmp, 1))) return false;
		
	if (bit_is_set(tmp, 6)) // 12H mode, must reset hours value
	{
		tmp &= ~(1<<6);
		if (!(ds1307_write(&address, &tmp, 1))) return false;
	}
	
	address = 0x00;
	// set ClockHalt (CH) to 0
	if (!(ds1307_read(&address, &tmp, 1))) return false;
	if (bit_is_set(tmp, 7))
	{
		tmp &= ~(1<<7);
		if (!(ds1307_write(&address, &tmp, 1))) return false;
	}
	
	// Enable SQWE at [see below]
	address = 0x07;
	tmp = 0b00010000;
	if (!(ds1307_write(&address, &tmp, 1))) return false;
	
	//Clock_SetUserData(0x07, 0b00000000); // disabled
	//Clock_SetUserData(0x07, 0b10010000); // 1Hz
	//Clock_SetUserData(0x07, 0b10010001); // 4.096kHz
	//Clock_SetUserData(0x07, 0b10010010); // 8.192kHz
	//Clock_SetUserData(0x07, 0b10010011); // 32.768kHz
	
	
	return true;
}

bool ds1307_read(uint8_t start_address, uint8_t *data, uint8_t bytes)
{	
	uint8_t reusult_code;
	reusult_code = usitwim_data_write(DS1307_ADDR, &start_address, 1); // ������ ������ ��������, ����� �������� ���������� ������� �� ������ �����
	if (reusult_code == USITWIM_RESULT_FAILURE) return false;
	
	reusult_code = usitwim_data_read(DS1307_ADDR, &data, bytes);
	if (reusult_code == USITWIM_RESULT_FAILURE) return false;
	
	return true;
}

bool ds1307_write(uint8_t start_address, uint8_t *data, uint8_t bytes)
{
	uint8_t reusult_code;
	reusult_code = usitwim_data_write(DS1307_ADDR, &start_address, 1); // ������ ������ ��������, ����� �������� ���������� ������� �� ������ �����
	if (reusult_code == USITWIM_RESULT_FAILURE) return false;
	reusult_code = usitwim_data_write(DS1307_ADDR, &data, bytes);
	if (reusult_code == USITWIM_RESULT_FAILURE) return false;
	
	return true;
}

//bool ds1307_read_three(uint8_t start_address, uint8_t *data_address)
//{
	//if(i2c_start(DS1307_ADDR + I2C_WRITE) != 0)	return false;	// ����� (������)
	//if(i2c_write(start_address) != 0)	return false;			// ������ ������ ��������, ����� �������� ���������� ������� �� ������ �����
	//i2c_stop();													// ���� (��� ������������ ������)
	//
	//i2c_start(DS1307_ADDR + I2C_READ);							// ����� (������) (��� i2c_start_wait, ������� ���������� �����)
	//data_address[0] = i2c_readAck();							// ������ ��� ���������� �� ��������, �� ������� ����� ���������� �������
	//data_address[1] = i2c_readAck();
	//data_address[2] = i2c_readNak();
	////i2c_stop();													// ���� (i2c_readNak() ��� ������ ����)
	//return true;
	//
//}