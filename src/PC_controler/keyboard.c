#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
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
#include "log.h"
#include "keyboard.h"
#include "communication_client_cmd.h"
#include "main.h"

int keyboard_init(void){
	/*
	 *
	 */
	int ret;
	int socket_fd, result;
	struct input_event kbd_event;

	bool cam_movement_maintain = false;

	int motor_right_action = 0;
	int motor_left_action = 0;
	int servo_cam_1_pos = 0;//relative ???
	int servo_cam_2_pos = 0;

	key_status ctrl_key_status={false};
	key_status old_key_status={false};

	socket_fd = open (g_pibot.app_option.input_keyboard, O_RDWR | O_NOCTTY | O_SYNC);
	if (socket_fd < 0)
	{
		common_die( -1, "error %d opening %s: %s", errno, g_pibot.app_option.input_keyboard , strerror (errno));
	}
	/* Socket has been created and connected to the other party */
	while(1){
		memset(&kbd_event, 0, sizeof(kbd_event));

		result = read(socket_fd, &kbd_event, sizeof(kbd_event));
		if (result == 0) {
			/* Event but read nothing*/
			printf("Event but read nothing");
		}
		else {
			/* Data readed manage it */
			//printf("Event read: type: %d, code: %d, value: %d\n", kbd_event.type, kbd_event.code, kbd_event.value);

			if(kbd_event.type == 1){ // Event key
				switch(kbd_event.code){
					//
					// PIBOT MOTOR CONTROL KEY
					//
					case KEY_Z_CODE: //Key_Z
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.Z_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								ctrl_key_status.Z_press = true;
								break;
						}
						break;
					case KEY_Q_CODE: //Key_Q
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.Q_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								ctrl_key_status.Q_press = true;
								break;
						}
						break;
					case KEY_S_CODE: //Key_S
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.S_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								ctrl_key_status.S_press = true;
								break;
						}
						break;
					case KEY_D_CODE: //Key_D
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.D_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								ctrl_key_status.D_press = true;
								break;
						}
						break;
						//
						// CAMERA CONTROL KEY
						//
					case KEY_I_CODE: //Key_I
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.I_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								cam_movement_maintain = true;
								ctrl_key_status.I_press = true;
								break;
						}
						break;
					case KEY_J_CODE: //Key_J
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.J_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								cam_movement_maintain = true;
								ctrl_key_status.J_press = true;
								break;
						}
						break;
					case KEY_K_CODE: //Key_K
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.K_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								cam_movement_maintain = true;
								ctrl_key_status.K_press = true;
								break;
						}
						break;
					case KEY_L_CODE: //Key_L
						switch(kbd_event.value){
							default:
							case E_KEY_ACTION_RELEASE: //Release
								ctrl_key_status.L_press = false;
								break;
							case E_KEY_ACTION_PUSH: //Push
							case E_KEY_ACTION_MAINTAIN: //Maintain
								cam_movement_maintain = true;
								ctrl_key_status.L_press = true;
								break;
						}
						break;
					default:
						//Don't care of the rest :p
						break;
				}
				//Check if status key have change since old one
				ret = memcmp(&ctrl_key_status, &old_key_status, sizeof(key_status));
				if((ret != 0)||(cam_movement_maintain == true)){
					//save status key
					cam_movement_maintain = false;
					old_key_status = ctrl_key_status;

					//Send command to pibot :p
					// 0 = Stop
					// 1 = Forward
					// 2 = Backward

					//
					// CONTROL MOTOR CHECK
					//
					if(ctrl_key_status.Z_press == true && ctrl_key_status.S_press == true){
						printf("Z and S not allowed simultaneous\n");
						motor_right_action = 0;
						motor_left_action = 0;
					}else if(ctrl_key_status.Q_press == true && ctrl_key_status.D_press == true){
						printf("Q and D not allowed simultaneous\n");
						motor_right_action = 0;
						motor_left_action = 0;
					}else{
						//all possible case double case
						if(ctrl_key_status.Z_press == true && ctrl_key_status.D_press == true){
							motor_right_action = 0;
							motor_left_action = 1;
						}else if(ctrl_key_status.Z_press == true && ctrl_key_status.Q_press == true){
							motor_right_action = 1;
							motor_left_action = 0;
						}else if(ctrl_key_status.S_press == true && ctrl_key_status.Q_press == true){
							motor_right_action = 2;
							motor_left_action = 0;
						}else if(ctrl_key_status.S_press == true && ctrl_key_status.D_press == true){
							motor_right_action = 0;
							motor_left_action = 2;
						}else{
							//all single case
							if(ctrl_key_status.Z_press == true){
								motor_right_action = 1;
								motor_left_action = 1;
							}else if(ctrl_key_status.Q_press == true){
								motor_right_action = 1;
								motor_left_action = 2;
							}else if(ctrl_key_status.S_press == true){
								motor_right_action = 2;
								motor_left_action = 2;
							}else if(ctrl_key_status.D_press == true){
								motor_right_action = 2;
								motor_left_action = 1;
							}else{
								printf("You shoudn't be here");
								motor_right_action = 0;
								motor_left_action = 0;
							}
						}
					}

					//
					// CONTROL CAMERA CHECK
					//
					if((ctrl_key_status.I_press == true) && (ctrl_key_status.K_press == true)){
						printf("I and K not allowed simultaneous\n");
						servo_cam_1_pos = 0;
						servo_cam_2_pos = 0;
					}else if((ctrl_key_status.J_press == true) && (ctrl_key_status.L_press == true)){
						printf("J and L not allowed simultaneous\n");
						servo_cam_1_pos = 0;
						servo_cam_2_pos = 0;
					}else{
						//all possible case double case
						if((ctrl_key_status.I_press == true) && (ctrl_key_status.L_press == true)){
							servo_cam_1_pos = -10;
							servo_cam_2_pos = -10;
						}else if((ctrl_key_status.I_press == true) && (ctrl_key_status.J_press == true)){
							servo_cam_1_pos = 10;
							servo_cam_2_pos = -10;
						}else if((ctrl_key_status.K_press == true) && (ctrl_key_status.J_press == true)){
							servo_cam_1_pos = 10;
							servo_cam_2_pos = 10;
						}else if((ctrl_key_status.K_press == true && ctrl_key_status.L_press == true)){
							servo_cam_1_pos = -10;
							servo_cam_2_pos = 10;
						}else{
							//all single case
							if(ctrl_key_status.I_press == true){
								servo_cam_1_pos = 0;
								servo_cam_2_pos = -10;
							}else if(ctrl_key_status.J_press == true){
								servo_cam_1_pos = 10;
								servo_cam_2_pos = 0;
							}else if(ctrl_key_status.K_press == true){
								servo_cam_1_pos = 0;
								servo_cam_2_pos = 10;
							}else if(ctrl_key_status.L_press == true){
								servo_cam_1_pos = -10;
								servo_cam_2_pos = 0;
							}else{
								printf("You shoudn't be here");
								servo_cam_1_pos = 0;
								servo_cam_2_pos = 0;
							}
						}
					}

					//Send data when all key are managed
					//printf("%s():l%d: Send data: Mot_R: %d, Mot_L: %d, Servo_1: %d, Servo_2: %d    \n",__FUNCTION__,__LINE__, motor_right_action, motor_left_action, servo_cam_1_pos, servo_cam_2_pos);
					ret = communication_client_cmd_send(motor_right_action, motor_left_action, servo_cam_1_pos, servo_cam_2_pos);
					//printf(ret, "Send command to pibot fail, return: %d", ret);
				}
			}
		}
	}

	return 0;
}
