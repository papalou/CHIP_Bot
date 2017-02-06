#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "log.h"
#include "main.h"
#include "pca9685.h"
#include "i2c_servo.h"

T_i2c_servo g_i2c_servo={
	.is_initialized = false,
};

int i2c_servo_set_pos_relative(int servo, int pos){
	int ret;
	int computed_pos = MIDDLE_SERVO_VALUE;

	if(g_i2c_servo.is_initialized == false){
		common_die(-1, "I2C servo module is not init");
	}

	switch(servo){
		case SERVO_CAM_1_ID:

			computed_pos = g_i2c_servo.servo_cam_1_pos + pos;
			printf("%s():l%d: Servo 1 computed pos raw: %d    \n",__FUNCTION__,__LINE__, computed_pos);

			if(computed_pos <= MIN_SERVO_VALUE){
				printf("Limit servo value relative cam 1: Min\n");
				computed_pos = MIN_SERVO_VALUE;
			}else if(computed_pos >= MAX_SERVO_VALUE){
				printf("Limit servo value relative cam 1: Max\n");
				computed_pos = MAX_SERVO_VALUE;
			}

			g_i2c_servo.servo_cam_1_pos = computed_pos;
			break;

		case SERVO_CAM_2_ID:

			computed_pos = g_i2c_servo.servo_cam_2_pos + pos;
			printf("%s():l%d: Servo 2 computed pos raw: %d    \n",__FUNCTION__,__LINE__, computed_pos);

			if(computed_pos <= MIN_SERVO_VALUE){
				printf("Limit servo value relative cam 2: Min\n");
				computed_pos = MIN_SERVO_VALUE;
			}else if(computed_pos >= MAX_SERVO_VALUE){
				printf("Limit servo value relative cam 2: Max\n");
				computed_pos = MAX_SERVO_VALUE;
			}

			g_i2c_servo.servo_cam_2_pos = computed_pos;
			break;

		default:
			common_die(-2, "wrong servo ID, you shoudn't be here");
			break;
	}

	printf("%s():l%d:RELATIVE Set Servo %d, computed Pos: %d , pos: %d   \n",__FUNCTION__,__LINE__, servo, computed_pos, pos);
	ret = pca9685_setPWM(&g_i2c_servo.pca9685_i2c_data, servo, computed_pos);
	common_die_zero(ret, -3, "error: pca9685_setPWM fail, return: %d", ret);

	return 0;
}

int i2c_servo_set_pos_absolue(int servo, int pos){
	int ret;

	if(g_i2c_servo.is_initialized == false){
		common_die(-1, "I2C servo module is not init");
	}

	if(pos <= MIN_SERVO_VALUE){
		printf("Limit servo value: Min\n");
		pos = MIN_SERVO_VALUE;
	}else if(pos >= MAX_SERVO_VALUE){
		printf("Limit servo value: Max\n");
		pos = MAX_SERVO_VALUE;
	}

	switch(servo){
		case SERVO_CAM_1_ID:
			g_i2c_servo.servo_cam_1_pos = pos;
			break;
		case SERVO_CAM_2_ID:
			g_i2c_servo.servo_cam_2_pos = pos;
			break;
		default:
			common_die(-2, "wrong servo ID, you shoudn't be here");
			break;
	}

	printf("%s():l%d: ABSOLUE Set Servo %d, Pos: %d     \n",__FUNCTION__,__LINE__, servo, pos);
	ret = pca9685_setPWM(&g_i2c_servo.pca9685_i2c_data, servo, pos);
	common_die_zero(ret, -3, "error: pca9685_setPWM fail, return: %d", ret);

	return 0;
}

int i2c_servo_init(){
	int ret;
	//Init bus I2C
	ret = pca9685_init(&g_i2c_servo.pca9685_i2c_data, 1,0x40);
	common_die_zero(ret, -1, "error: pca9685_init fail, return: %d", ret);
	usleep(1000 * 100);

	//Setting frequency
	ret = pca9685_setPWMFreq(&g_i2c_servo.pca9685_i2c_data, 61);
	common_die_zero(ret, -2, "error: pca9685_ fail, return: %d", ret);
	usleep(1000 * 1000);

	//Init the initial pos of the two servo
	ret = pca9685_setPWM(&g_i2c_servo.pca9685_i2c_data, SERVO_CAM_1_ID, MIDDLE_SERVO_VALUE);
	common_die_zero(ret, -3, "error: pca9685_setPWM fail, return: %d", ret);
	g_i2c_servo.servo_cam_1_pos = MIDDLE_SERVO_VALUE;

	ret = pca9685_setPWM(&g_i2c_servo.pca9685_i2c_data, SERVO_CAM_2_ID, MIDDLE_SERVO_VALUE);
	common_die_zero(ret, -4, "error: pca9685_setPWM fail, return: %d", ret);
	g_i2c_servo.servo_cam_2_pos = MIDDLE_SERVO_VALUE;

	g_i2c_servo.is_initialized = true;

	return 0;
}
