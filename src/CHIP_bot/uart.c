#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"
#include "log.h"
#include "uart.h"

static int _set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		common_die( -1, "error %d from tcgetattr", errno);
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	//disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		common_die( -1, "error %d from tcsetattr", errno);
	}
	return 0;
}

static int _set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		common_die( -1, "error %d from tggetattr", errno);
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		common_die( -1, "error %d setting term attributes", errno);

	return 0;
}

int uart_init(char * path, int *fd, int blocking){
	int ret;
	
	common_die_null(path, -1, "error: path is null");
	common_die_null(fd, -2, "error: fd is null");

	*fd = open (path, O_RDWR | O_NOCTTY | O_SYNC);
	if (*fd < 0)
	{
		common_die( -1, "error %d opening %s: %s", errno, path, strerror (errno));
	}

	ret = _set_interface_attribs(*fd, B115200, 0);
	common_die_zero(ret, -3, "error: set uart attribute fail, return: %d", ret);

	ret = _set_blocking(*fd, blocking);
	common_die_zero(ret, -4, "error: set blocking fail, return: %d", ret);

	return 0;
}

