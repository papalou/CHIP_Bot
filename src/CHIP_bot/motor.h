#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "gpio.h"

//TO CHECK
#define GPIO_MOTOR_LEFT_FORWARD 9
#define GPIO_MOTOR_LEFT_BACKWARD 11

#define GPIO_MOTOR_RIGHT_FORWARD 8
#define GPIO_MOTOR_RIGHT_BACKWARD 7

#define GPIO_STANBY_CONTROLER 25

typedef struct{
	bool is_initialized;
	T_gpio gpio_left_motor_forward;
	T_gpio gpio_left_motor_backward;
	T_gpio gpio_right_motor_forward;
	T_gpio gpio_right_motor_backward;
	T_gpio gpio_stanby_controler;
} T_motor;

int motor_set_motor_cmd(uint8_t cmd_right_motor, uint8_t cmd_left_motor);
int motor_init(void);
#endif
