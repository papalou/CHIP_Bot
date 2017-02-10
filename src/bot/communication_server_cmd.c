#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "common.h"
#include "crc.h"
#include "motor.h"
#include "i2c_servo.h"
#include "thread.h"
#include "log.h"
#include "main.h"
#include "common_pibot_communication.h"
#include "communication_server_cmd.h"

T_com_server_cmd g_server_cmd={
	.is_initialized = false,
	.socket_fd_server_cmd = -1,
};

static int _thread_server_cmd(T_thread * thread){
	int ret;
	struct sockaddr_in dest;
	int client_fd;
	socklen_t size;
	T_com_packet packet;

	common_die_null(thread, -1, "error: thread data is null");

	memset(&dest,0,sizeof(dest));

	while(1) {

		size = sizeof(struct sockaddr_in);

		client_fd = accept(g_server_cmd.socket_fd_server_cmd, (struct sockaddr *)&dest, &size);
		common_die_zero(client_fd, -2, "error: accept fail, return: %d", ret);

		printf("Server cmd got connection from client %s\n", inet_ntoa(dest.sin_addr));

		while(1) {

			ret = recv(client_fd, &packet, sizeof(T_com_packet), 0);
			log_zero(ret, "Receive packet fail, return: %d", ret);
			if (ret == 0) {
				printf("Connection closed force motor stop\n");
				ret = motor_set_motor_cmd(0, 0);
				log_zero(ret, "error set motor command fail, return: %d", ret);
				break;
			}

			printf("receive data from client\n");
			ret = crc_compute_8(&packet.data, sizeof(T_com_data));
			if(ret != packet.crc){
				log_sat("error: crc is not correct; computed: %d, packet: %d", ret, packet.crc);
			}else{
				ret = motor_set_motor_cmd(packet.data.motor_right, packet.data.motor_left);
				log_zero(ret, "error set motor command fail, return: %d", ret);

				ret = i2c_servo_set_pos_relative(SERVO_CAM_1_ID, packet.data.servo_cam_1_pos);
				log_zero(ret, "error set servo relative pos command fail, return: %d", ret);
				
				ret = i2c_servo_set_pos_relative(SERVO_CAM_2_ID, packet.data.servo_cam_2_pos);
				log_zero(ret, "error set servo relative pos command fail, return: %d", ret);
			}

			//printf("Server:Msg Received %s\n", buffer);
			//if ((send(client_fd,buffer, strlen(buffer),0))== -1)
			//{
			//	fprintf(stderr, "Failure Sending Message\n");
			//	close(client_fd);
			//	break;
			//}

			//printf("Server:Msg being sent: %s\nNumber of bytes sent: %d\n",buffer, strlen(buffer));

		} //End of Inner While...
		//Close Connection Socket
		close(client_fd);
	} //Outer While

}

int communication_init_server_cmd(){
	int ret;
	struct sockaddr_in server;
	int yes =1;

	if ((g_server_cmd.socket_fd_server_cmd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
		common_die(-1, "Socket failure!!");
	}

	ret = setsockopt(g_server_cmd.socket_fd_server_cmd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	common_die_zero(ret, -2, "setsockopt fail, return: %d", ret);

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(SOCKET_CMD_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	if ((bind(g_server_cmd.socket_fd_server_cmd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    { //sizeof(struct sockaddr)
		fprintf(stderr, "Binding Failure\n");
		exit(1);
	}

	if ((listen(g_server_cmd.socket_fd_server_cmd, SOCKET_CMD_BACKLOG))== -1){
		fprintf(stderr, "command Listening Failure\n");
		exit(1);
	}

	ret = thread_create_with_priority ( &g_server_cmd.server_cmd_thread, "Thread reception cmd", _thread_server_cmd, NULL, PRIORITY_NICE_LOW);
	common_die_zero(ret, -12, "error: create reception thread fail, return: %d", ret);

	g_server_cmd.is_initialized = true;

	return 0;

}
