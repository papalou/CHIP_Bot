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
	printf("	-h, --help    : help mode\n");
	printf("	-v, --verbose : verbose mode\n");
	printf("\n");

	return 0;
}
