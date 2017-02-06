#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <stdio.h>      /* Standard I/O functions */
#include <fcntl.h>
#include <syslog.h>		/* Syslog functionallity */
#include <inttypes.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
//

#include "pca9685.h"

//! Open device file for PCA9685 I2C bus
/*!
 \return fd returns the file descriptor number or -1 on error
 */
static int openfd(T_pca9685 * pca9685) {
	int fd;
	if ((fd = open(pca9685->busfile, O_RDWR)) < 0) {
		printf ("Couldn't open I2C Bus %d [openfd():open %d]", pca9685->_i2cbus, errno);
		return -1;
	}
	if (ioctl(fd, I2C_SLAVE, pca9685->_i2caddr) < 0) {
		printf ("I2C slave %d failed [openfd():ioctl %d]", pca9685->_i2caddr, errno);
		return -1;
	}

	return fd;
}

//! Write a single byte from PCA9685
/*!
 \param fd file descriptor for I/O
 \param address register address to write to
 \param data 8 bit data to write
 */
static int write_byte(int fd, uint8_t address, uint8_t data) {
	uint8_t buff[2];
	buff[0] = address;
	buff[1] = data;
	if (write(fd, buff, sizeof(buff)) != 2) {
		printf("Failed to write to I2C Slave 0x%x @ register 0x%x [write_byte():write %d]", /*_i2caddr*/0x00, address, errno);
		usleep(5000);
	}else{
		//printf("Wrote to I2C Slave 0x%x @ register 0x%x [0x%x]\n", _i2caddr, address, data);
	}

	return 0;
}

//! Read a single byte from PCA9685
/*!
 \param fd file descriptor for I/O
 \param address register address to read from
 */
static uint8_t read_byte(int fd, uint8_t address) {

	return 0;

	uint8_t dataBuffer[BUFFER_SIZE];
	uint8_t buff[BUFFER_SIZE];
	buff[0] = address;
	if (write(fd, buff, BUFFER_SIZE) != BUFFER_SIZE) {
		printf("I2C slave 0x%x failed to go to register 0x%x [read_byte():write %d]", /*_i2caddr*/0x00, address, errno);
		return (-1);
	} else {
		if (read(fd, dataBuffer, BUFFER_SIZE) != BUFFER_SIZE) {
			printf ("Could not read from I2C slave 0x%x, register 0x%x [read_byte():read %d]", /*_i2caddr*/0x00, address, errno);
			return (-1);
		}
	}

	return 0;
}

//! Sets PCA9685 mode to 00
int pca9685_reset(T_pca9685 * pca9685) {
	int fd = openfd(pca9685);
	if (fd != -1) {
		write_byte(fd, MODE1, 0x00); //Normal mode
		write_byte(fd, MODE2, 0x04); //Normal mode
		close(fd);
	}
	return 0;
}
//! Set the frequency of PWM
/*!
 \param freq desired frequency. 40Hz to 1000Hz using internal 25MHz oscillator.
 */
int pca9685_setPWMFreq(T_pca9685 * pca9685, int freq) {
	int fd = openfd(pca9685);
	if (fd != -1) {
		uint8_t prescale = (CLOCK_FREQ / 4096 / freq)  - 1;
		printf ("Setting prescale value to: %d\n", prescale);
		printf ("Using Frequency: %d\n", freq);

		uint8_t oldmode = read_byte(fd, MODE1);
		uint8_t newmode = (oldmode & 0x7F) | 0x10;    //sleep
		write_byte(fd, MODE1, newmode);        // go to sleep
		write_byte(fd, PRE_SCALE, prescale);
		write_byte(fd, MODE1, oldmode);
		usleep(10*1000);
		write_byte(fd, MODE1, oldmode | 0x80);

		close(fd);
	}
	return 0;
}

//! PWM a single channel
/*!
 \param led channel to set PWM value for
 \param value 0-4095 value for PWM
 */
int pca9685_setPWM(T_pca9685 * pca9685, uint8_t led, int value) {
	pca9685_setPWM_custom(pca9685, led, 0, value);
	return 0;
}
//! PWM a single channel with custom on time
/*!
 \param led channel to set PWM value for
 \param on_value 0-4095 value to turn on the pulse
 \param off_value 0-4095 value to turn off the pulse
 */
int pca9685_setPWM_custom(T_pca9685 * pca9685, uint8_t led, int on_value, int off_value) {
	int fd = openfd(pca9685);
	if (fd != -1) {

		write_byte(fd, LED0_ON_L + LED_MULTIPLYER * led, on_value & 0xFF);

		write_byte(fd, LED0_ON_H + LED_MULTIPLYER * led, on_value >> 8);

		write_byte(fd, LED0_OFF_L + LED_MULTIPLYER * led, off_value & 0xFF);

		write_byte(fd, LED0_OFF_H + LED_MULTIPLYER * led, off_value >> 8);

		close(fd);
	}

	return 0;
}


//! Constructor takes bus and address arguments
/*!
 \param bus the bus to use in /dev/i2c-%d.
 \param address the device address on bus
 */
int pca9685_init(T_pca9685 * pca9685, int bus, int address) {
	pca9685->_i2cbus = bus;
	pca9685->_i2caddr = address;
	snprintf(pca9685->busfile, sizeof(pca9685->busfile), "/dev/i2c-%d", bus);
	pca9685_reset(pca9685);
	return 0;
}

