#include "cmd_interp.h"
#include "cmd_util.h"
#include "../main.h"


/*Code module from AVR command shell project.
See appropriate header file for detais.*/

uint8_t* arg_ptr[MAX_ARGS_NUM];
uint8_t args_num;

//Splitting arguments with zeroes and putting their start
//addresses to array / storing their total number
void split_args(uint8_t* cmdline)
{
	uint8_t i;

	args_num=0;
	i=0;

	while ((i < MAX_CMD_LEN) && (cmdline[i] != 0) && (args_num < MAX_ARGS_NUM + 1))
	{
		i++;

		if ((is_regular_char(cmdline[i])) && (cmdline[i-1]==' '))
		//if ((is_digit(cmdline[i])) && (cmdline[i-1]==' '))
		{
			arg_ptr[args_num]=&(cmdline[i]);
			args_num++;
			cmdline[i-1] = 0;
		}
	}

	//for (i=0; i<MAX_CMD_LEN; i++)
	//{
		//if (cmdline[i]==' ')	cmdline[i]=0;
	//}
}

//Extract and start appropriate fn
uint8_t cmd_exec(uint8_t *cmdline)
{
	split_args(cmdline);

	if (str_equal(cmdline, "date"))
	{	
		if (args_num == 0)
		{
			//TIO_TextOutput("don't know yet\r\n");
			usart_sendString("cgy\r\n");
			return 0;
		}
		else 
		{
			if (args_num == 1)
			{
				usart_sendString(arg_ptr[0]);
				usart_sendString("\r\n");
			} 
			//else if (args_num == 3)
			//{
				//for (i=0; i<args_num; i++)
				//{
					//usart_sendString(arg_ptr[i]);
					//usart_sendString("\r\n");
				//}
			//} 
			else
			{
				usart_sendString("hlp\r\n");
				//usart_sendString("date [YY MM DD");
				//usart_sendString("|DayOfWeek]\r\n");
			}
		}
		return 1;
	}
	
	return 0;
}