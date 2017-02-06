#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>

#include "common.h"
#include "log.h"
#include "help.h"
#include "motor.h"
#include "i2c_servo.h"
#include "communication_server_cmd.h"
#include "communication_server_video.h"
#include "main.h"

#define UART_PORT "/dev/rfcomm0"

T_pibot g_pibot={.is_initialized = false,
				 .uart_fd = -1,
				};

static int _get_opt(int argc, char** argv)
{
	int option;
	int option_index = 0;
	static struct option long_options[] =
	{
		/* These options set a flag. */
		{"verbose", no_argument,       0, 'v'},
		{"help"   , no_argument,       0, 'h'},
		{        0,           0,       0,   0}
	};

	while (1)
	{

		option = getopt_long (argc, argv, "vh", long_options, &option_index);

		/* Detect the end of the options. */
		if (option == -1)
			break;

		switch (option)
		{
			case 'v':
				g_pibot.app_option.verbose_mode = true;
				break;
			case 'h':
			case '?':
			default:
				return HELP_START_MODE;
				break;
		}
	}
	return NORMAL_START_MODE;
}

int main(int argc, char** argv)
{
	int ret;
	printf("Pibot start\n");

	ret = _get_opt(argc, argv);
	common_die_zero(ret, -1, "Error: get opt fail, return: %d", ret);

	//Add init here

	g_pibot.is_initialized = true;

	switch(ret)
	{
		case NORMAL_START_MODE:
			break;
		case HELP_START_MODE:
		default:
				ret = help_show_help();
				common_die_zero(ret, -2, "Error: show help fail, return: %d", ret);
				return 0; //Exit the app
			break;
	}

	ret = motor_init();
	common_die_zero(ret, -3, "error: init uart fail, return: %d", ret);

	ret = i2c_servo_init();
	common_die_zero(ret, -4, "error: init uart fail, return: %d", ret);

	ret = communication_init_server_cmd();
	common_die_zero(ret, -5, "error init communication server socket, return: %d", ret);

//	ret = communication_init_server_video();
//	common_die_zero(ret, -6, "error init communication server video socket, return: %d", ret);


	while(1)
	{
		pause();
	}
	return 0;
}


