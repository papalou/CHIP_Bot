#ifndef __COMMUNICATION_CLIENT_CMD_H__
#define __COMMUNICATION_CLIENT_CMD_H__

typedef struct{
	bool is_initialized;
	int socket_fd_client_cmd;
}T_com_client_cmd;

int communication_init_client_cmd(void);
int communication_client_cmd_send(int motor_right, int motor_left, int servo_cam_1_pos, int servo_cam_2_pos);

#endif
