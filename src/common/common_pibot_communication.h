#ifndef __COMMON_PIBOT_COMMUNICATION_H__
#define __COMMON_PIBOT_COMMUNICATION_H__

#define SOCKET_CMD_PORT   3490
#define SOCKET_VIDEO_PORT 3491
#define SOCKET_CMD_BACKLOG 10
#define SOCKET_VIDEO_BACKLOG 10

#define CAM_WIDTH  320
#define CAM_HEIGHT 240

//#define CAM_WIDTH  640
//#define CAM_HEIGHT 480

//#define MAX_VIDEO_BUFFER_SIZE ((CAM_WIDTH * CAM_HEIGHT) * 2)
#define MAX_VIDEO_BUFFER_SIZE 81920 //MJPEG for 320*240

#define START_OF_FRAME 0x88

typedef enum{
	E_stop = 0,
	E_forward,
	E_backward,
	//E_force_break,
}E_direction;

typedef struct{
	uint8_t motor_right;
	uint8_t motor_left;
	int8_t  servo_cam_1_pos;
	int8_t  servo_cam_2_pos;
}T_com_data;

typedef struct{
	T_com_data data;
	uint8_t crc;
}T_com_packet;

typedef struct{
	uint32_t data_length;
	uint8_t header_crc;
}NOT_ALIGNED T_com_video_header;

typedef struct{
	uint8_t start_of_frame;
	T_com_video_header header;
	uint8_t data[MAX_VIDEO_BUFFER_SIZE];
}NOT_ALIGNED T_com_video_packet;

#endif
