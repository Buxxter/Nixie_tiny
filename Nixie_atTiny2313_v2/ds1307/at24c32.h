#ifndef AT24C32_H_
#define AT24C32_H_


#include <stdbool.h>
#include "i2cmaster.h"

#define AT24C32_ADDR 0b10101110


//#define bit_is_clear(port, bit)		(!(port & (1<<bit)))
//#define bit_is_set(port, bit)		(port & (1<<bit))

bool at24c32_init(void);
bool at24c32_read(uint8_t address_h, uint8_t address_l, uint8_t *data, uint8_t bytes);
bool at24c32_write(uint8_t address_h, uint8_t address_l, uint8_t *data, uint8_t bytes);



#endif /* AT24C32_H_ */