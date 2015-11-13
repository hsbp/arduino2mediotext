#include "Remote.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFLEN 512
#define HANDLE_BUF(D, X, E) \
	write(to_remote[1], "K", 1);\
	X;\
	bytes = read(from_remote[0], buf, BUFLEN);\
	printf("[%s] ", D);\
	if (memcmp(buf, E, bytes) != 0) printf("NOK "); else printf("OK ");\
	for (int i = 0; i < bytes; i++) {\
		printf("%02hhx", buf[i]);\
	}\
	putchar('\n');

int main(int argc, char **argv) {
	int to_remote[2], from_remote[2];
	int bytes;
	char buf[BUFLEN];
	pipe(to_remote);
	pipe(from_remote);
	HANDLE_BUF("test_init", Remote r(to_remote[0], from_remote[1]),
			"\xC7\x0C\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")
	r.setPixel(0, 0, true);
	HANDLE_BUF("test_first_block", r.flushPixels(), "\xC1\x01\x80")
	r.setPixel(8, 0, true);
	HANDLE_BUF("test_second_block", r.flushPixels(), "\xC1\x12\x80");
	r.setPixel(9, 0, true);
	HANDLE_BUF("test_second_block", r.flushPixels(), "\xC1\x12\xc0");
	r.flushPixels();
	printf("--- done ---\n");
	return 0;
}
