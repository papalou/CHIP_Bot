#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//#include "common.h"
#include "log.h"
#include "crc.h"
#include "common_pibot_communication.h"
#include "communication_client_cmd.h"
#include "main.h"

T_com_client_cmd g_client_cmd={
	.is_initialized = false,
	.socket_fd_client_cmd = -1,
};

int communication_client_cmd_send(int motor_right, int motor_left, int servo_cam_1_pos, int servo_cam_2_pos){

	int ret;
	T_com_packet packet;

	if(g_client_cmd.is_initialized == false){
		common_die(-1, "error: can't send data com is not initialized");
	}

	packet.data.motor_right = motor_right;
	packet.data.motor_left = motor_left;
	packet.data.servo_cam_1_pos = servo_cam_1_pos;
	packet.data.servo_cam_2_pos = servo_cam_2_pos;
	packet.crc = crc_compute_8(&packet.data, sizeof(T_com_data));
	printf("Data crc: %d\n", packet.crc);

	ret = send(g_client_cmd.socket_fd_client_cmd, &packet, sizeof(T_com_packet), 0);
	common_die_zero(ret, -2, "error: send command data fail, return: %d", ret);
	return 0;
}

int communication_init_client_cmd(void){

	int ret;
	struct sockaddr_in server_info;
	struct hostent *he;
//	char buffer[1024];
//	char buff[1024];

	if ((he = gethostbyname(g_pibot.app_option.pibot_ip))==NULL) {
		common_die(-1, "Cannot get host name");
	}

	if ((g_client_cmd.socket_fd_client_cmd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
		common_die(-2, "Socket Failure!!");
	}

	memset(&server_info, 0, sizeof(server_info));
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(SOCKET_CMD_PORT);
	server_info.sin_addr = *((struct in_addr *)he->h_addr);

	ret = connect(g_client_cmd.socket_fd_client_cmd, (struct sockaddr *)&server_info, sizeof(struct sockaddr));
	common_die_zero(ret, -3, "error: connect fail, return: %d", ret);

	g_client_cmd.is_initialized = true;

//	while(1) {
//		printf("Client: Enter Data for Server:\n");
//		fgets(buffer,CMD_MAXSIZE-1,stdin);
//		if ((send(g_client_cmd.socket_fd_client_cmd,buffer, strlen(buffer),0))== -1) {
//			fprintf(stderr, "Failure Sending Message\n");
//			close(g_client_cmd.socket_fd_client_cmd);
//			exit(1);
//		}
//		else {
//			printf("Client:Message being sent: %s\n",buffer);
//			ret = recv(g_client_cmd.socket_fd_client_cmd, buffer, sizeof(buffer),0);
//			if ( ret <= 0 )
//			{
//				printf("Either Connection Closed or Error\n");
//				//Break from the While
//				break;
//			}
//
//			buff[ret] = '\0';
//			printf("Client:Message Received From Server -  %s\n",buffer);
//		}
//	}
//	close(g_client_cmd.socket_fd_client_cmd);
//
	return 0;
}
