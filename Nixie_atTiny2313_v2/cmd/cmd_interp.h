#ifndef CMD_INTERP_H_
#define CMD_INTERP_H_

#include <stdint.h>
#include "../main.h"

//#define MAX_CMD_LEN         25
#define MAX_ARGS_NUM        3

//Command interpreting function.
//Takes command line and executes appropriate handler.
uint8_t cmd_exec(uint8_t* cmdline);



#endif /* CMD_INTERP_H_ */