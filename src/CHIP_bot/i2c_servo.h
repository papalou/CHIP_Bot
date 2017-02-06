#ifndef __I2C_SERVO_H__
#define __I2C_SERVO_H__

#include "pca9685.h"

#define SERVO_CAM_1_ID 0 //right to left
#define SERVO_CAM_2_ID 1 //bottom to top

#define MIN_SERVO_VALUE 160
#define MIDDLE_SERVO_VALUE 375
#define MAX_SERVO_VALUE 600

typedef struct{
	int is_initialized;
	T_pca9685 pca9685_i2c_data;
	uint16_t servo_cam_1_pos; // between 160 and 600 otherwise you break the servo...
	uint16_t servo_cam_2_pos;
}T_i2c_servo;

int i2c_servo_init(void);
int i2c_servo_set_pos_relative(int servo, int pos);
int i2c_servo_set_pos_absolue(int servo, int pos);

#endif
