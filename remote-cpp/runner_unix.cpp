#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
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

#include "Remote.h"
#include "runner.h"

#define SOCKET_PATH "simul8r.sock"

void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char **argv) {
	struct sockaddr_un remote;
	int len;
	const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) error("Unix socket open");
	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCKET_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(fd, (struct sockaddr *)&remote, len) == -1) error("Unix connect");

	Remote r(fd, fd);
	run(r);
}
