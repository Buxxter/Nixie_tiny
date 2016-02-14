#ifndef DS1307_H_
#define DS1307_H_

#include <stdbool.h>
#include "usitwim.h"

#define DS1307_ADDR 0XD0

//#define bit_is_clear(port, bit)		(!(port & (1<<bit)))
//#define bit_is_set(port, bit)		(port & (1<<bit))

bool ds1307_init(void);
bool ds1307_read(uint8_t start_address, uint8_t *data, uint8_t bytes);
bool ds1307_write(uint8_t start_address, uint8_t *data, uint8_t bytes);

#endif /* DS1307_H_ */