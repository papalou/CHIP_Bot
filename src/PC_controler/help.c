#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "help.h"

int help_show_help()
{
	printf("\n");
	printf("= = = Help mode = = =\n");
	printf("\n");
	printf("list of option:\n");
	printf("	-d, --ip      : ip of the pibot AP (default: '192.168.2.1')\n");
	printf("	-k, --event_kb: input event keyboard (default: '/dev/input/event0')\n");
	printf("	-h, --help    : help mode\n");
	printf("	-v, --verbose : verbose mode\n");
	printf("\n");

	return 0;
}
