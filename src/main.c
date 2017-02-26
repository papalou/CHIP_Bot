#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>

#include "log.h"

#include "main.h"

T_app g_app={
				.is_initialized = false,
			};

static int _usage(void){
	printf("Usage:\n");
	printf("\n");
	printf(" -m, --mode <arg> : chose mode between 0 (bot) and 1 (controler) \n");
	printf("\n");
	printf(" -v, --verbose    : active verbose mode\n");
	printf(" -h, --help       : show help (this)\n");
	printf("\n");
	return 0;
}

static int _get_opt(int argc, char** argv)
{
	int ret;
	int option;
	int option_index = 0;
	static struct option long_options[] =
	{
		/* These options set a flag. */
		{     "mode", required_argument,       0, 'm'},
		{  "verbose",       no_argument,       0, 'v'},
		{     "help",       no_argument,       0, 'h'},
		{          0,                 0,       0,   0}
	};

	while (1)
	{
		option = getopt_long (argc, argv, "m:vh", long_options, &option_index);

		/* Detect the end of the options. */
		if (option == -1)
			break;

		switch (option)
		{
			case 'm':
				g_app.app_option.start_mode = atoi(optarg);
				break;
			case 'v':
				g_app.app_option.verbose_mode = true;
				break;
			case 'h':
			case '?':
			default:
				ret = _usage();
				common_die_negative(ret, -1, "Error, show _usage fail, return: %d", ret);
				break;
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	int ret;
	printf("Application Start\n");

	//Init libcommon log
	ret = libcommon_init_log();
	if(ret < 0){
		printf("Error init log fail, return: %d, we can't continu, ABORT", ret);
		return -1;
	}

	//Make log use color for more lisibility
	ret = libcommon_set_color_log( true);
	common_die_negative(ret, -2, "Error: libcommon_set_color_log fail, return: %d", ret);

	ret = _get_opt( argc, argv);
	common_die_negative( ret, -3, "Error: parse _get_opt fail, return: %d", ret);

	switch(g_app.app_option.start_mode){
		case E_MODE_BOT: //0
			write_log( "Start App in E_MODE_BOT");
			break;
		case E_MODE_CONTROLER: //1
			write_log( "Start App in E_MODE_CONTROLER");
			break;
		default:
			common_die( -4, "Error wrong start mode: %d", g_app.app_option.start_mode);
			break;
	}

	while(1)
	{
		pause();
	}
	return 0;
}


