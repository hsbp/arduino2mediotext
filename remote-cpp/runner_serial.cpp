#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "Remote.h"
#include "runner.h"

#define BAUDRATE B19200
#define PORT "/dev/ttyUSB0"

void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char **argv) {
	struct termios oldtio, newtio;
	int fd;

	fd = open(PORT, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		perror(PORT);
		return 1;
	}

	tcgetattr(fd, &oldtio);

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	
	sleep(2);
	Remote r(fd, fd);
	run(r);

	usleep(0);
	tcsetattr(fd, TCSANOW, &oldtio);
	return 0;
}
