// Delay Controller module
#include "typedefs.h"
#include <compat/deprecated.h>
#include <avr/io.h>
//#include "controller.h"
#define nop() asm volatile ("nop")

void dummyloop(unsigned int);

void dummyloop(unsigned int timetoloop)
{ 
  while (timetoloop>0) {
			nop();
			timetoloop--;
  }
}


