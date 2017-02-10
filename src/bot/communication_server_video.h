#ifndef __COMMUNICATION_SERVER_VIDEO_H__
#define __COMMUNICATION_SERVER_VIDEO_H__

typedef struct{
	bool is_initialized;
	int socket_fd_server_video;
	T_thread server_video_thread;
}T_com_server_video;

int communication_init_server_video();

#endif
