#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define BIT_PER_BYTE 8
#define TCP_PORT 11328
#define ACK "ACK\n"
#define MEM_ERROR "MEM_ERROR\n"

/* display settings */
#define ROWS 7
#define COLS 96

/* serial port settings */
#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUDRATE B19200

#define FRAME_BYTES (COLS / BIT_PER_BYTE * ROWS)
#define OUTPUT_BUFLEN (FRAME_BYTES + 2)

typedef uint8_t frame[FRAME_BYTES];

/* Global variables */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
frame* live_frames = NULL;
uint16_t live_frame_count = 0;
uint16_t live_frame_wait = 0;

void error(char *msg) {
	perror(msg);
	exit(1);
}

uint16_t recv_short(const int fd) {
	uint16_t retval;

	while (read(fd, (char *)&retval, 1) < 1);
	while (read(fd, ((char *)&retval + 1), 1) < 1);

	return ntohs(retval);
}

void recv_frame(const int fd, frame dest) {
	int bytes = sizeof(frame);
	int offset = 0;
	while (bytes) {
		int n = read(fd, (char *)dest + offset, bytes);
		offset += n;
		bytes -= n;
	}
}

int open_display() {
#ifdef SOCKET_PATH
	struct sockaddr_un remote;
	int len;
	const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) error("Unix socket open");
	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCKET_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(fd, (struct sockaddr *)&remote, len) == -1) error("Unix connect");
#else
	struct termios newtio;
	const int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
	if (fd < 0) error("serial open");

	sleep(2);

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
#endif
	return fd;
}

void* display_thread() {
	const int fd = open_display();
	char outbuf[OUTPUT_BUFLEN];
	uint16_t current_frame = 0;
	
	outbuf[0] = 0xC0 | ROWS;
	outbuf[1] = COLS / BIT_PER_BYTE;

	while (1) {
		int frame_bytes = OUTPUT_BUFLEN, frame_offset = 0;

		pthread_mutex_lock(&mutex);
		char invalid_frames = live_frame_count == 0;
		pthread_mutex_unlock(&mutex);

		if (invalid_frames) {
			usleep(100000);
			continue;
		}

		pthread_mutex_lock(&mutex);
		if (current_frame >= live_frame_count) current_frame = 0;
		memcpy(outbuf + 2, &live_frames[current_frame], sizeof(frame));
		useconds_t sleep_time = live_frame_wait * 1000;
		pthread_mutex_unlock(&mutex);

		while (frame_bytes) {
			int sent = write(fd, outbuf + frame_offset, frame_bytes);
			frame_bytes -= sent;
			frame_offset += sent;
		}

		while (read(fd, &frame_offset, 1) != 1) usleep(10);
		current_frame++;

		usleep(sleep_time);
	}
}

void* network_thread() {
	int listen_sock;
	int accept_sock;
	unsigned int clientlen;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	int optval;

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) error("ERROR opening socket");

	optval = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,
			(const void *)&optval , sizeof(int));

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(TCP_PORT);

	if (bind(listen_sock, (struct sockaddr *) &serveraddr,
				sizeof(serveraddr)) < 0) error("ERROR on binding");

	if (listen(listen_sock, 5) < 0) error("ERROR on listen");

	clientlen = sizeof(clientaddr);
	while (1) {
		uint16_t frame_count, frame_wait, frame_no;
		frame* frames;

		accept_sock = accept(listen_sock, (struct sockaddr *) &clientaddr, &clientlen);
		if (accept_sock < 0) error("ERROR on accept");

		frame_count = recv_short(accept_sock);
		frame_wait = recv_short(accept_sock);

		frames = (frame*)malloc(frame_count * sizeof(frame));

		if (frames) {
			for (frame_no = 0; frame_no < frame_count; frame_no++) {
				recv_frame(accept_sock, frames[frame_no]);
			}

			pthread_mutex_lock(&mutex);
			if (live_frames != NULL) free(live_frames);
			live_frames = frames;
			live_frame_count = frame_count;
			live_frame_wait = frame_wait;
			pthread_mutex_unlock(&mutex);

			write(accept_sock, ACK, strlen(ACK));
		} else {
			write(accept_sock, MEM_ERROR, strlen(MEM_ERROR));
		}

		close(accept_sock);
	}
}

int main() {
	pthread_t disp_t, net_t;

	pthread_create(&net_t, NULL, network_thread, NULL);
	pthread_create(&disp_t, NULL, display_thread, NULL);
	pthread_join(net_t, NULL);
	pthread_join(disp_t, NULL);
	return 0;
}
