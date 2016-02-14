#ifndef CMD_UTIL_H_
#define CMD_UTIL_H_

/*
Useful routines, mainly char/string processing.

Written by YS from radiokot.ru and we.easyelectronics.ru

You may freely use and redistribute this code any way you want,
but only at your own risk. Author gives no warranty and takes no
responsibility on anything that could happen while using this code.
*/

#include <stdint.h>
#include <avr/pgmspace.h>

//Returns 1 if supplied character is letter or digit.
uint8_t is_regular_char(char x);
//The same if only digit.
uint8_t is_digit(char x);
//Returns length of string in RAM
uint8_t str_len(uint8_t* str);
//Returns length of string in FLASH
uint8_t str_len_pgm(uint8_t* pgm_str);
//Compares str in RAM with pgm_str in flash
//returns 1 if they are equal.
uint8_t str_equal_pgm(uint8_t* str,uint8_t* pgm_str);

uint8_t str_equal(uint8_t* str1, char* str2);

//Converts numeric string to 16-bit integer.
//NO ERROR HANDLING, so, say, 70000 will be succesfully
//converted, but, of course, to incorrect value.
uint16_t str_to_uint16(uint8_t *s_num);

#endif

