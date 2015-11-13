#include "Remote.h"
#include <stdio.h>

int main(int argc, char **argv) {
	Remote r(0, 1);
	r.setPixel(0, 0, true);
	r.flushPixels();
	r.setPixel(8, 0, true);
	r.flushPixels();
	r.setPixel(9, 0, true);
	r.flushPixels();
	r.flushPixels();
	/*printf("---flushpixel1---\n");
	r.flushPixels();
	printf("---setpixel---\n");
	r.setPixel(0, 0, true);
	printf("---flushpixel2---\n");
	r.flushPixels();
	printf("---setpixel---\n");
	r.setPixel(8, 3, false);
	r.setPixel(1, 1, false);
	printf("---flushpixel3---\n");
	r.flushPixels();*/
	return 0;
}
