#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SDL/SDL.h"	/*SDL*/
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_image.h"

//#include "common.h"
#include "statemachine.h"
#include "log.h"
#include "crc.h"
#include "thread.h"
#include "common_pibot_communication.h"
#include "communication_client_video.h"
#include "main.h"

T_com_client_video g_client_video={
	.is_initialized = false,
	.socket_fd_client_video = -1,
};

static int _handler_state_start  (T_statemachine *sm );
static int _handler_state_init   (T_statemachine *sm );
static int _handler_state_data   (T_statemachine *sm );
static int _handler_state_header (T_statemachine *sm );

static int _check_true           (T_statemachine *sm );

enum{
	STATE_UART_INIT = 10,
	STATE_UART_START,
	STATE_UART_HEADER,
	STATE_UART_DATA,
};

static int _show_receive_image(void){
	//SDL_LockYUVOverlay(g_client_video.screen_data.data);

	//memcpy(g_client_video.screen_data.data->pixels[0], &g_client_video.video_packet.data, sizeof(g_client_video.video_packet.data));

	//SDL_UnlockYUVOverlay(g_client_video.screen_data.data);
	///* displaying the YUV data on the screen */
	//SDL_DisplayYUVOverlay(g_client_video.screen_data.data, &g_client_video.screen_data.rect);
	
	// Create a stream based on our buffer.
	g_client_video.screen_data.buffer_stream = SDL_RWFromMem(&g_client_video.video_packet.data, MAX_VIDEO_BUFFER_SIZE);

	// Create a surface using the data coming out of the above stream.
	g_client_video.screen_data.frame = IMG_Load_RW(g_client_video.screen_data.buffer_stream, 0);

	// Blit the surface and flip the screen.
	SDL_BlitSurface(g_client_video.screen_data.frame, NULL, g_client_video.screen_data.screen, &g_client_video.screen_data.rect);
	SDL_Flip(g_client_video.screen_data.screen);

	return 0;
}

static T_state socket_receive_states[] = {

	STATE_BEGIN ( STATE_UART_INIT )
		{ _handler_state_init         , STATE_UART_INIT   },
		{ _check_true                 , STATE_UART_START  },
	STATE_END

	STATE_BEGIN ( STATE_UART_START )
		{ _handler_state_start        , STATE_UART_HEADER },
		{ _check_true                 , STATE_UART_INIT   },
	STATE_END

	STATE_BEGIN ( STATE_UART_HEADER )
		{ _handler_state_header       , STATE_UART_DATA   },
		{ _check_true                 , STATE_UART_INIT   },
	STATE_END

	STATE_BEGIN ( STATE_UART_DATA )
		{ _handler_state_data         , STATE_UART_INIT   },
		{ _check_true                 , STATE_UART_INIT   },
	STATE_END
};

static int _check_true(T_statemachine *UNUSED(sm) )
{
    return 1;
}


static int _handler_state_init(T_statemachine *sm )
{
	//int ret;

	common_die_null(sm,0,"null pointer");

	if(statemachine_get_nbloop(sm) != 1){
		usleep(100000);
		return 0;
	}

	debug_add("%s():l%d: INIT STATE MACHINE    \n",__FUNCTION__,__LINE__); 

    return 1;

}

/* ****************************************************************************/
/**
* @brief handler get start of file
* read uart, get SOF, if all good go to next step get header.
*
* @param [in] UNUSED(nb_loop)   :   usefull to make the handler action one time
*
* @return 0   : ok
* @return < 0 : error
*/
/* ****************************************************************************/
static int _handler_state_start(T_statemachine *UNUSED(sm) )
{
    int nb_read = 0;
	T_com_video_packet * frame = &g_client_video.video_packet;

	do{
		nb_read = recv(g_client_video.socket_fd_client_video, &frame->start_of_frame, sizeof(uint8_t), 0); //Read 1 Bytes
		debug_add("receive %d bytes of data\n", nb_read);
		if ( nb_read < 0 ){
			write_log("read error!%s",strerror(errno));
			/*
			 * Warrning !! little hack :
			 * to avoid burst loop in the state machine,
			 * we set a timer here, in case the read fail at each time
			 */
			usleep(100000);
			return 0;
		}

		if(nb_read != 1){
			debug_add("read errornb_read = %d\n",nb_read);
			return 0;
		}
	}
	while(frame->start_of_frame != START_OF_FRAME);

	debug_add("%s():l%d:_______ SOF is FOUND ______    \n",__FUNCTION__,__LINE__); 
	return 1;
}

/* ****************************************************************************/
/**
* @brief handler get header
* read uart, get header, check crc, if all good go to next step get data.
*
* @param [in] UNUSED(nb_loop)   :   usefull to make the handler action one time
*
* @return 0   : ok
* @return < 0 : error
*/
/* ****************************************************************************/
static int _handler_state_header(T_statemachine *UNUSED(sm) )
{
	int nb_read;
	uint32_t nb_read_sum;
	uint8_t crc;
	T_com_video_packet *  frame = &g_client_video.video_packet;
	uint8_t            * buffer = (uint8_t*)(&frame->header);

	nb_read_sum = 0;

	while(nb_read_sum < sizeof(T_com_video_header))	{

		nb_read = recv(g_client_video.socket_fd_client_video, &buffer[nb_read_sum],sizeof(T_com_video_header) - nb_read_sum, 0);
		common_die_zero(nb_read,0,"read error!%s",strerror(errno));

		if(nb_read == 0)
            common_die(0,"timeout");

		nb_read_sum += nb_read;
	}

	if(nb_read_sum != sizeof(T_com_video_header))
		common_die(0,"warrning !! you must not be here");

	crc = crc_compute_8((uint8_t*)frame,offsetof(T_com_video_packet,header.header_crc));

	if(frame->header.header_crc != crc )
		common_die(0,"bad crc : %02x != %02x",frame->header.header_crc,crc);

	debug_add("%s():l%d:_______ HEADER FOUND________    \n",__FUNCTION__,__LINE__); 
	return 1;
}

/* ****************************************************************************/
/**
* @brief handler get data
* read uart, get data, check crc, if all good go to next step process data.
*
* @param [in] UNUSED(nb_loop)   :   usefull to make the handler action one time
*
* @return 0   : ok
* @return < 0 : error
*/
/* ****************************************************************************/
static int _handler_state_data(T_statemachine *UNUSED(sm) )
{

	int       ret;
    uint32_t  nb_read_sum;
    int       nb_read;
	int       size_data_to_get;
	T_com_video_packet * frame = &g_client_video.video_packet;

	size_data_to_get = frame->header.data_length;
	nb_read_sum = 0;

	if(size_data_to_get != 0){

		while(nb_read_sum < (frame->header.data_length)){

			nb_read = recv(g_client_video.socket_fd_client_video, &frame->data[nb_read_sum],size_data_to_get - nb_read_sum, 0);
			common_die_zero(nb_read,0,"read error!%s",strerror(errno));

			if(nb_read == 0)
				common_die(0,"timeout ");

			nb_read_sum += nb_read;
		}
	}

	debug_add("%s():l%d: ______ALL DATA FOUND______    \n",__FUNCTION__,__LINE__); 

	ret = _show_receive_image();
	common_die_zero(ret, 1, "Error: show image fail, continue anyway");
	return 1;
}


static int _thread_client_video(T_thread * thread){
	int ret;

	common_die_null(thread, -1, "error: thread data is null");
	printf("Thread video reception start\n");
	
	while(1){
		ret = statemachine_wakeup(&g_client_video.socket_statemachine);
		write_log(ret,"error in statemachine_wakeup : %d",ret);
	}
	return 0;
}

static int _init_sdl_video(void){
	/* initializing SDL */
	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		//goto close_video;
	}

	IMG_Init(IMG_INIT_JPG);

	/* Set up the screen */
	g_client_video.screen_data.screen = SDL_SetVideoMode(CAM_WIDTH, CAM_HEIGHT, 32, SDL_HWSURFACE);

	/* If there was in error in setting up the screen */
	if(g_client_video.screen_data.screen == NULL)
	{
		printf("failed to set up screen\n");
		//goto close_sdl;
	}


	/* Set the window caption */
	SDL_WM_SetCaption("pibot live stream -> Ok framerate suck", NULL);
	/* Setting YUV OVERLAY */
	//g_client_video.screen_data.data = SDL_CreateYUVOverlay(CAM_WIDTH, CAM_HEIGHT, SDL_YUY2_OVERLAY, g_client_video.screen_data.screen);
	g_client_video.screen_data.rect.x = 0;
	g_client_video.screen_data.rect.y = 0;
	g_client_video.screen_data.rect.w = CAM_WIDTH;
	g_client_video.screen_data.rect.h = CAM_HEIGHT;
	
	//if(g_client_video.screen_data.data == NULL)
	//{
	//	printf("failed to set up data\n");
	//	//goto close_sdl;
	//}
	return 0;
}

int communication_init_client_video(void){

	int ret;
	struct sockaddr_in server_info;
	struct hostent *he;

	g_client_video.screen_data.screen = NULL;
	g_client_video.screen_data.data = NULL;

	ret = _init_sdl_video();
	common_die_zero(ret, -1, "Error: _init_sdl_video fail, return: %d", ret);
	
	if ((he = gethostbyname(g_pibot.app_option.pibot_ip))==NULL) {
		common_die(-1, "Cannot get host name");
	}

	if ((g_client_video.socket_fd_client_video = socket(AF_INET, SOCK_STREAM, 0))== -1) {
		common_die(-2, "Socket Failure!!");
	}

	memset(&server_info, 0, sizeof(server_info));
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(SOCKET_VIDEO_PORT);
	server_info.sin_addr = *((struct in_addr *)he->h_addr);
			
	ret = connect(g_client_video.socket_fd_client_video, (struct sockaddr *)&server_info, sizeof(struct sockaddr));
	common_die_zero(ret, -3, "error: connect fail, return: %d", ret);
	
	ret = statemachine_init( &g_client_video.socket_statemachine, socket_receive_states, sizeof(socket_receive_states)/sizeof(T_state), "Socket_receive_frame", STATE_UART_INIT, true, NULL);
    common_die_zero( ret, -6, "Unable to init statemachine:%d", ret);
	
	ret = statemachine_disable_loop_detection( &g_client_video.socket_statemachine);
    common_die_zero( ret, -7, "Unable to disable loop detection:%d", ret);
	
	ret = thread_create_with_priority ( &g_client_video.client_video_thread, "Thread reception video", _thread_client_video, NULL, PRIORITY_NICE_LOW);
	common_die_zero(ret, -5, "error: create reception thread fail, return: %d", ret);

	g_client_video.is_initialized = true;

	return 0;
}
