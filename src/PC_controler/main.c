#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <linux/input.h>
#include <linux/uinput.h>

//#include "common.h"
#include "keyboard.h"
#include "communication_client_cmd.h"
#include "communication_client_video.h"
#include "log.h"
#include "help.h"
#include "main.h"

#define DEFAULT_IP "192.168.2.1"
#define DEFAULT_INPUT_DEVICE "/dev/input/event0"

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
		{       "ip", required_argument,       0, 'i'},
		{ "input_kb", required_argument,       0, 'k'},
		{  "verbose",       no_argument,       0, 'v'},
		{     "help",       no_argument,       0, 'h'},
		{          0,                 0,       0,   0}
	};

	//Set default option value
	snprintf(g_pibot.app_option.pibot_ip, 20, "%s", DEFAULT_IP);
	snprintf(g_pibot.app_option.input_keyboard, 50, "%s", DEFAULT_INPUT_DEVICE);

	while (1)
	{

		option = getopt_long (argc, argv, "vhi:", long_options, &option_index);

		/* Detect the end of the options. */
		if (option == -1)
			break;

		switch (option)
		{
			case 'i':
				snprintf(g_pibot.app_option.pibot_ip, 20, "%s", optarg);
				printf("Custom ip set: '%s'\n", g_pibot.app_option.pibot_ip);
				break;
			case 'k':
				snprintf(g_pibot.app_option.input_keyboard, 50, "%s", optarg);
				printf("Custom Input keyboard set: '%s'\n", g_pibot.app_option.input_keyboard);
				break;
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

/*
 *
 * MAIN
 *
 */

int main(int argc, char** argv)
{
	int ret;

	printf("Pibot controler start\n");

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
			common_die_zero(ret, -1, "Error: show help fail, return: %d", ret);
			return 0; //Exit the app
			break;
	}

	ret = communication_init_client_cmd();
	common_die_zero(ret, -3, "Init communication command socket fail, return: %d", ret);
	printf("Init cmd communication done...\n");

//	ret = communication_init_client_video();
//	common_die_zero(ret, -4, "Init communication video socket fail, return: %d", ret);
//	printf("Init video communication done...\n");

	ret = keyboard_init();
	common_die_zero(ret, -5, "error: init keyboard fail, return: %d", ret);
	printf("Init keyboard control done...\n");
	/*
	 *
	 */

	printf("Pibot controler quit...");

	return 0;
}

