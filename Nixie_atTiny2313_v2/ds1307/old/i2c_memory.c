
// EEPROM with i2c module
// 2010 Stas Novoselsky

#include <avr/interrupt.h>
#include <delay_.h>
#include <compat/deprecated.h>

#include "controller.h"
#include "usi_i2c.h"

#define USI_DELAY 50

void i2c_mem_init(void);
uint8_t i2c_mem_write(uint8_t chip_adr, unsigned int mem_adr, uint8_t *data_, uint8_t data_size);
uint8_t i2c_mem_read(uint8_t chip_adr, unsigned int mem_adr, uint8_t *data_, uint8_t data_size);
uint8_t i2c_mem_current_read(uint8_t chip_adr, uint8_t *data_, uint8_t data_size);
uint8_t i2c_mem_pages_fill_byte(uint8_t chip_adr, unsigned int page_adr, uint8_t data_byte);

void i2c_mem_init(void) 
{
	uint8_t i3;

	init_usi_i2c_master();
	i2c_stop();
	dummyloop(USI_DELAY);
	for (i3=0;i3<9;i3++) {
		USIDR=0xff;
		cbi(sclport,scl);
		dummyloop(USI_DELAY);
		sbi(sclport,scl);
		dummyloop(USI_DELAY);
	};
}

uint8_t i2c_mem_write(uint8_t chip_adr, unsigned int mem_adr, uint8_t *data_, uint8_t data_size_) 
{

	uint8_t write_buf[4], *aaa, stat_;

	write_buf[0]=(chip_adr&0xfe);
	do {

		if ((bit_is_clear(sclportinp,scl))||(bit_is_clear(sdaportinp,sda))) return (1);
		aaa=(unsigned char *)&mem_adr;
		write_buf[2]=(*aaa);
		aaa++;
		write_buf[1]=(*aaa);
		write_buf[3]=(*data_);
		i2c_start();
		dummyloop(USI_DELAY);
		stat_=0;
		switch (i2c_master_send(write_buf,4)) {

			case 1: {		// Error SCL
				stat_=2;
				break;
			};
			case 2: {		// NASC
				stat_=3;
				break;
			};
			default : {		// Ok
				break;
			};
		};

		dummyloop(USI_DELAY);
		i2c_stop();
		if (stat_==0) {
			data_++;
			data_size_--;
			mem_adr++;
		}
		else {
			if (stat_!=3) {
				return (stat_);
			};

		};
		dummyloop(USI_DELAY);
	}
	while (data_size_>0);
	return(0);

}

uint8_t i2c_mem_read(uint8_t chip_adr, unsigned int mem_adr, uint8_t *data_, uint8_t data_size_) 
{

	uint8_t write_buf[3], *aaa;

	write_buf[0]=(chip_adr&0xfe);
	aaa=(unsigned char *) &mem_adr;
	write_buf[2]=(*aaa);
	aaa++;
	write_buf[1]=(*aaa);
	if ((bit_is_clear(sclportinp,scl))||(bit_is_clear(sdaportinp,sda))) return (1);
	i2c_start();
	dummyloop(USI_DELAY);
	switch (i2c_master_send(write_buf,3)) {

		case 1: {
			return (2); // Error SCL
		};
		case 2: {
			return (3); // NASC
		};
		default: {
			break; // Ok
		};

	};
	write_buf[0]=(chip_adr|0x01);
	dummyloop(USI_DELAY);
	i2c_start(); // second start
	switch (i2c_master_send(write_buf,1)) {

		case 1: {
			return(4); //Error SCL
		};
		case 2: {
			return(5); // NASC
		};
		default: {
			break; // Ok
		};
	};
	dummyloop(USI_DELAY);
	switch (i2c_master_read(data_,data_size_)) {

		case 1: {
			return (6);
		};
		default: {
			break;
		};
	};
	dummyloop(USI_DELAY);
	i2c_stop();
	return(0);
} 

uint8_t i2c_mem_current_read(uint8_t chip_adr, uint8_t *data_, uint8_t data_size_) 
{

	uint8_t write_buf[2];

	if ((bit_is_clear(sclportinp,scl))||(bit_is_clear(sdaportinp,sda))) return (1);
	write_buf[0]=(chip_adr|0x01);
	dummyloop(USI_DELAY);
	i2c_start(); // second start
	switch (i2c_master_send(write_buf,1)) {

		case 1: {
			return(4); //Error SCL
		};
		case 2: {
			return(5); // NASC
		};
		default: {
			break; // Ok
		};
	};
	dummyloop(USI_DELAY);
	switch (i2c_master_read(data_,data_size_)) {

		case 1: {
			return (6);
		};
		default: {
			break;
		};
	};
	dummyloop(USI_DELAY);
	i2c_stop();
	return(0);
}


uint8_t i2c_mem_pages_fill_byte(uint8_t chip_adr, unsigned int page_adr, uint8_t data_byte) 
{

	uint8_t write_buf[4], *aaa;
	unsigned int mem_adr,kk;

	#ifdef AT24C64
		#define PAGESIZE 32
		#define MAXPAGES 256
	#else
		#ifdef AT24C256
			#define PAGESIZE 64
			#define MAXPAGES 512
		#else
			#define PAGESIZE 64
			#define MAXPAGES 512
		#endif
	#endif


	for (kk=page_adr; kk<MAXPAGES ; kk++) {
		write_buf[0]=(chip_adr&0xfe);
		if ((bit_is_clear(sclportinp,scl))||(bit_is_clear(sdaportinp,sda))) return (1);
		mem_adr=(kk*PAGESIZE);
		aaa = (unsigned char *)&mem_adr;
		write_buf[2]=(*aaa);
		aaa++;
		write_buf[1]=(*aaa);
		write_buf[3]=data_byte;
		i2c_start();
		dummyloop(USI_DELAY);
		switch (i2c_master_send(write_buf,4)) {

			case 1: {		// Error SCL
				return (2);
			};
			case 2: {		// NASC
				return (3);
			};
			default: {		// Ok
				break;
			};
		};
		dummyloop(USI_DELAY);
		write_buf[0]=data_byte;
		for (mem_adr=1; mem_adr<PAGESIZE; mem_adr++) {
			switch (i2c_master_send(write_buf,1)) {

				case 1: {	// Error SCL
					return (4);
				};
				case 2: {	// NASC
					return (5);
				};
				default: {	// Ok
					break;
				};
			};
		};
		dummyloop(USI_DELAY);
		i2c_stop();
		dummyloop(USI_DELAY*6);
	};
	return (0);

}



// ----------------  end of file   -------------------------
