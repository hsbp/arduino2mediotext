#include <unistd.h>
#include <stdio.h>

#include "Remote.h"
#include "runner.h"

void run(Remote &r) {
	for (uint8_t y = 0; y < ROWS; y++) {
		for (uint8_t x = 0; x < COLS; x++) {
			printf("x = %hhu y = %hhu\n", x, y);
			r.setPixel(x, y, true);
			r.flushPixels();
			usleep(50000);
			r.setPixel(x, y, false);
		}
	}
}
