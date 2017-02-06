#ifndef __COMMUNICATION_CLIENT_VIDEO_H__
#define __COMMUNICATION_CLIENT_VIDEO_H__

#include <stdbool.h>
#include "SDL/SDL.h"	/*SDL*/
#include "SDL/SDL_mixer.h"

#include "thread.h"
#include "statemachine.h"
#include "common_pibot_communication.h"

#define IMAGE_WINDOWS_NAME "pibot_stream"

typedef struct{
	SDL_Surface *screen;
	SDL_Overlay *data;
	SDL_Rect rect;

	SDL_RWops* buffer_stream;
	SDL_Surface* frame;

}T_screen_sdl_data;

typedef struct{
	bool is_initialized;
	int socket_fd_client_video;
	T_thread client_video_thread;
	T_statemachine socket_statemachine;
	T_com_video_packet video_packet; //actual video packet
	T_screen_sdl_data screen_data;
}T_com_client_video;

int communication_init_client_video(void);

#endif
