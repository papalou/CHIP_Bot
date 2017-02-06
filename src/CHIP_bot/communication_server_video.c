#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>  /* Video for Linux Two */


#include "common.h"
#include "crc.h"
#include "motor.h"
#include "i2c_servo.h"
#include "thread.h"
#include "log.h"
#include "main.h"
#include "common_pibot_communication.h"
#include "communication_server_video.h"

T_com_server_video g_server_video={
	.is_initialized = false,
	.socket_fd_server_video = -1,
};

struct buffer {
	void *start;
	size_t length;
};

struct dev_info {
	int fd;
	int format;
	struct buffer *buffers;
	int n_buffers;
};

int init_mmap(struct dev_info * device)
{
	struct v4l2_requestbuffers req;
	struct v4l2_buffer cur_buf;
	int i,n_buffers;
	enum v4l2_buf_type type;

	memset(&req, 0, sizeof(req));

	/* initiate memory mapping usign IOCTL */
	/* setting the buffers count to 4 */
	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/* specifying the memory type to MMAP */
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == ioctl (device->fd, VIDIOC_REQBUFS, &req)) {
		printf("failed to initiate memory mapping (%d)\n", errno);
		return -1;
	}

	/* if device doesn't supprt multiple buffers */
	if (req.count < 2) {
		printf("device doesn't support multiple buffers(%d)\n",
				req.count);
		return -1;
	}


	/* allocating buffers struct*/
	device->buffers = calloc (req.count, sizeof (struct buffer));
	if (!(device->buffers)) {
		printf("failed to allocate buffers\n");
		return -1;
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		/* using IOCTL to query the buffer status */
		memset(&cur_buf, 0, sizeof(cur_buf));
		cur_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cur_buf.memory      = V4L2_MEMORY_MMAP;
		cur_buf.index       = n_buffers;

		if (-1 == ioctl (device->fd, VIDIOC_QUERYBUF, &cur_buf)) {
			printf("failed to query buffer status %d\n", errno);
			return -1;
		}

		/* mmapping the buffers */
		device->buffers[n_buffers].length = cur_buf.length;
		device->buffers[n_buffers].start =
			mmap(NULL, cur_buf.length, PROT_READ | PROT_WRITE,
					MAP_SHARED, device->fd, cur_buf.m.offset);
		if (MAP_FAILED == device->buffers[n_buffers].start) {
			printf("failed to map buffer\n");
			return -1;
		}
	}

	/* enquiing buffers to device using IOCTL */
	for (i = 0; i < n_buffers; ++i) {
		//struct v4l2_buffer buf;

		memset(&cur_buf, 0, sizeof(cur_buf));
		cur_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cur_buf.memory      = V4L2_MEMORY_MMAP;
		cur_buf.index       = i;

		if (-1 == ioctl (device->fd, VIDIOC_QBUF, &cur_buf))
			return -1;
	}

	device->n_buffers = n_buffers;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* starting the streaming */
	if (-1 == ioctl (device->fd, VIDIOC_STREAMON, &type)) {
		printf("failed to start the streaming (%d)\n", errno);
		return -1;
	}

	return 1;
}

int init_video_device(struct dev_info *device)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	//unsigned int min;


	/* using IOCTL to quesry the device capabilities */
	if (-1 == ioctl(device->fd, VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno) {
			printf ("device is no V4L2 device\n");
			return -1;
		} else {
			printf("Error getting device capabilities (%d)\n",
					errno);
			return -1;
		}
	}

	/* checking if the device supports video capture */
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		printf("device is no video capture device\n");
		return -1;
	}

	/* checking if the device supports video streaming */
	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("device does not support streaming i/o\n");
		return -1;
	}


	/* Using IOCTL to query the capture capabilities */
	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl (device->fd, VIDIOC_CROPCAP, &cropcap))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		/* using IOCTL to set the cropping rectengle */
		if (-1 == ioctl (device->fd, VIDIOC_S_CROP, &crop))
		{
			printf("failed to set cropping rectengle\n");
		}
	}


	/* setting the video data format */
	memset(&fmt, 0, sizeof(fmt));
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = CAM_WIDTH;
	fmt.fmt.pix.height      = CAM_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

	if (-1 == ioctl (device->fd, VIDIOC_S_FMT, &fmt)) {
		printf ("Failed to set video data format\n");
		return -1;
	}

	/* initalizing device memory */
	if (init_mmap (device) < 0) {
		printf("device dosen't suppprt MMAP\n");
		return -1;
	}
	return 1;

}

int read_frame(struct dev_info *device, void *buffer, unsigned int buffer_size)
{
	struct v4l2_buffer buf;

	//memset(&buf, 0, sizeof(buf));

	/* using IOCTL to dequeue a full buffer from the queue */
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(device->fd, VIDIOC_DQBUF, &buf)) {
		printf("failed to dequeue buffer (%d)\n", errno);
		return -1;
	}

	printf("%s():l%d: buffer length: %d    \n",__FUNCTION__,__LINE__, device->buffers[buf.index].length);
	if( device->buffers[buf.index].length > buffer_size){
		common_die(-3, "error: buffer is to small, data size: %d, buffer size: %d", device->buffers[buf.index].length, buffer_size);
	}

	/* coping the frame data */
	memcpy(buffer, device->buffers[buf.index].start,
			device->buffers[buf.index].length);

	/* using IOCTL to enqueue back the buffer after we used it */
	if (-1 == ioctl(device->fd, VIDIOC_QBUF, &buf)) {
		printf("failed to enqueue buffer (%d)\n", errno);
		return -1;
	}

	return 1;
}

int open_video_device(const char *device_name)
{
	struct stat st;
	int fd;

	if (-1 == stat (device_name, &st)) {
		printf("stat failed\n");
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		printf ( "device is no char device\n");
		return -1;
	}

	fd = open(device_name, O_RDWR , 0);
	if (-1 == fd) {
		printf ( "Cannot open device\n");
		return -1;
	}
	return fd;
}

void close_video_device(struct dev_info *device)
{
	int i;
	for(i = 0; i < device->n_buffers; i++) {
		if (device->buffers) {
			munmap(device->buffers[i].start,
					device->buffers[i].length);
		}
	}
	if (device->buffers) {
		free(device->buffers);
	}
	close(device->fd);
}

static int _send_all(int socket, void *buffer, size_t length)
{
	int i;
	char *ptr = (char*) buffer;
	while (length > 0)
	{
		i = send(socket, ptr, length, 0);
		if (i < 1){
			return -1;
		}
		debug_add("%s():l%d: Send: %d    \n",__FUNCTION__,__LINE__, i);
		ptr += i;
		length -= i;
	}
	return 0;
}

static int _thread_server_video(T_thread * thread){
	int ret;
	struct sockaddr_in dest;
	int client_fd;
	socklen_t size;
	T_com_video_packet video_packet;
	struct dev_info vid_dev;

	common_die_null(thread, -1, "error: thread data is null");
	memset(&dest,0,sizeof(dest));

	//Initialize camera
	memset(&vid_dev, 0, sizeof(vid_dev));
	/* initializing video device - use your own device instead */
	vid_dev.fd = open_video_device("/dev/video0");
	common_die_zero(vid_dev.fd, -2, "failed to open video device");

	ret = init_video_device(&vid_dev);
	common_die_zero(ret, -3, "failed to initalize video device");

	printf("%s():l%d: Thread video will start    \n",__FUNCTION__,__LINE__);
	while(1) {

		size = sizeof(struct sockaddr_in);

		client_fd = accept(g_server_video.socket_fd_server_video, (struct sockaddr *)&dest, &size);
		common_die_zero(client_fd, -2, "error: accept fail, return: %d", ret);

		printf("Server video got connection from client %s\n", inet_ntoa(dest.sin_addr));

		while(1) {
			debug_add("%s():l%d: read camera frame   \n",__FUNCTION__,__LINE__);
			printf("%s():l%d:  FRAME.....   \n",__FUNCTION__,__LINE__); 
			ret = read_frame(&vid_dev, &video_packet.data, sizeof(video_packet.data));
			if(ret<0)
			{
				fprintf( stderr, "!!! read frame failed!\n" );
				break;
			}

			//Init packet:
			video_packet.start_of_frame = START_OF_FRAME;
			video_packet.header.data_length = MAX_VIDEO_BUFFER_SIZE;

			//Compute CRC
			video_packet.header.header_crc = crc_compute_8((uint8_t*)&video_packet,offsetof(T_com_video_packet,header.header_crc));

			debug_add("Server: Send video packet, size: %d\n", sizeof(video_packet));
			ret = _send_all(client_fd, &video_packet, sizeof(T_com_video_packet));
			if(ret < 0){
				fprintf(stderr, "Failure Sending video Message\n");
				close(client_fd);
				//And stop recording camera
				break;
			}
//			usleep(100000); //100ms

		}//End of Inner While...
		//Close Connection Socket

		close(client_fd);
	} //Outer While

	return 0;
}

int communication_init_server_video(){
	int ret;
	struct sockaddr_in server;
	int yes =1;

	if ((g_server_video.socket_fd_server_video = socket(AF_INET, SOCK_STREAM/*SOCK_DGRAM*/, 0))== -1) {
		common_die(-1, "Socket failure!!");
	}

	ret = setsockopt(g_server_video.socket_fd_server_video, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	common_die_zero(ret, -2, "setsockopt fail, return: %d", ret);

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(SOCKET_VIDEO_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	if ((bind(g_server_video.socket_fd_server_video, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    { //sizeof(struct sockaddr)
		fprintf(stderr, "Binding Failure\n");
		exit(1);
	}

	if ((listen(g_server_video.socket_fd_server_video, SOCKET_VIDEO_BACKLOG))== -1){
		fprintf(stderr, "Video Listening Failure\n");
		exit(1);
	}

	ret = thread_create_with_priority ( &g_server_video.server_video_thread, "Thread reception video", _thread_server_video, NULL, PRIORITY_NICE_LOW);
	common_die_zero(ret, -12, "error: create reception thread fail, return: %d", ret);

	g_server_video.is_initialized = true;

	return 0;

}
