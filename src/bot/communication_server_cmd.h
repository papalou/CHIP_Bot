#ifndef __COMMUNICATION_SERVER_CMD_H__
#define __COMMUNICATION_SERVER_CMD_H__

#define CMD_PORT 3490
#define CMD_BACKLOG 10

typedef struct{
	bool is_initialized;
	int socket_fd_server_cmd;
	T_thread server_cmd_thread;
}T_com_server_cmd;

int communication_init_server_cmd();

#endif
