#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "log.h"
#include "common_pibot_communication.h"
#include "motor.h"

T_motor g_motor = {.is_initialized = false};

int motor_set_motor_cmd(uint8_t cmd_right_motor, uint8_t cmd_left_motor){
	int ret;

	if(g_motor.is_initialized == false){
		common_die(-1, "set motor not possible, is not init");
	}

	printf("New motor command:\nRight motor: %d \nLeft  motor: %d \n", cmd_right_motor, cmd_left_motor);

	switch(cmd_right_motor)
	{
		case E_forward:
			ret = gpio_output_set( &g_motor.gpio_right_motor_forward, 1);
			common_die_zero(ret, -2, "set gpio fail, return: %d", ret);

			ret = gpio_output_set( &g_motor.gpio_right_motor_backward, 0);
			common_die_zero(ret, -3, "set gpio fail, return: %d", ret);
			break;
		case E_backward:
			ret = gpio_output_set( &g_motor.gpio_right_motor_forward, 0);
			common_die_zero(ret, -2, "set gpio fail, return: %d", ret);

			ret = gpio_output_set( &g_motor.gpio_right_motor_backward, 1);
			common_die_zero(ret, -3, "set gpio fail, return: %d", ret);
			break;
		default:
		case E_stop:
			ret = gpio_output_set( &g_motor.gpio_right_motor_forward, 0);
			common_die_zero(ret, -2, "set gpio fail, return: %d", ret);

			ret = gpio_output_set( &g_motor.gpio_right_motor_backward, 0);
			common_die_zero(ret, -3, "set gpio fail, return: %d", ret);
			break;
	}
	
	switch(cmd_left_motor)
	{
		case E_forward:
			ret = gpio_output_set( &g_motor.gpio_left_motor_forward, 1);
			common_die_zero(ret, -4, "set gpio fail, return: %d", ret);

			ret = gpio_output_set( &g_motor.gpio_left_motor_backward, 0);
			common_die_zero(ret, -5, "set gpio fail, return: %d", ret);
			break;
		case E_backward:
			ret = gpio_output_set( &g_motor.gpio_left_motor_forward, 0);
			common_die_zero(ret, -4, "set gpio fail, return: %d", ret);

			ret = gpio_output_set( &g_motor.gpio_left_motor_backward, 1);
			common_die_zero(ret, -5, "set gpio fail, return: %d", ret);
			break;
		default:
		case E_stop:
			ret = gpio_output_set( &g_motor.gpio_left_motor_forward, 0);
			common_die_zero(ret, -4, "set gpio fail, return: %d", ret);

			ret = gpio_output_set( &g_motor.gpio_left_motor_backward, 0);
			common_die_zero(ret, -5, "set gpio fail, return: %d", ret);
			break;
	}

	return 0;
}

int motor_init(void){
	int ret;

	ret = gpio_init_output( &g_motor.gpio_left_motor_forward, GPIO_MOTOR_LEFT_FORWARD, 0);
	common_die_zero(ret, -1, "init gpio fail, return: %d", ret);

	ret = gpio_init_output( &g_motor.gpio_left_motor_backward, GPIO_MOTOR_LEFT_BACKWARD, 0);
	common_die_zero(ret, -2, "init gpio fail, return: %d", ret);

	ret = gpio_init_output( &g_motor.gpio_right_motor_forward, GPIO_MOTOR_RIGHT_FORWARD, 0);
	common_die_zero(ret, -3, "init gpio fail, return: %d", ret);

	ret = gpio_init_output( &g_motor.gpio_right_motor_backward, GPIO_MOTOR_RIGHT_BACKWARD, 0);
	common_die_zero(ret, -4, "init gpio fail, return: %d", ret);

	ret = gpio_init_output( &g_motor.gpio_stanby_controler, GPIO_STANBY_CONTROLER, 1);
	common_die_zero(ret, -5, "init gpio fail, return: %d", ret);

	g_motor.is_initialized = true;
	return 0;
}
