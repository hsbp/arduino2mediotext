#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Remote.h"
#include "runner.h"

void run(Remote &r) {
	srand(time(NULL));

	for (uint8_t y = 0; y < ROWS; y++) {
		for (uint8_t x = 0; x < COLS; x++) {
			if (rand() % 3 == 0) r.setPixel(x, y, true);
		}
	}

	for (int step = 0; step < 100; step++) {
		usleep(200000);
		uint8_t queue[COLS * ROWS];
		memset(queue, 0, sizeof(queue));
		for (uint8_t y = 0; y < ROWS; y++) {
			for (uint8_t x = 0; x < COLS; x++) {
				uint8_t living =
					r.getPixel((x - 1) % COLS, y) +
					r.getPixel((x + 1) % COLS, y) +
					r.getPixel((x - 1) % COLS, (y - 1) % ROWS) +
					r.getPixel((x + 1) % COLS, (y + 1) % ROWS) +
					r.getPixel((x - 1) % COLS, (y + 1) % ROWS) +
					r.getPixel((x + 1) % COLS, (y - 1) % ROWS) +
					r.getPixel(x % COLS, (y - 1) % ROWS) +
					r.getPixel(x % COLS, (y + 1) % ROWS);
				bool old_state = r.getPixel(x, y);
				bool new_state = (living == 2 && old_state) || living == 3;
				if (old_state != new_state) queue[y * COLS + x] = 2 | new_state;
			}
		}
		for (uint8_t y = 0; y < ROWS; y++) {
			for (uint8_t x = 0; x < COLS; x++) {
				const uint8_t cell = queue[y * COLS + x];
				if (cell) r.setPixel(x, y, (cell & 1) != 0);
			}
		}
		r.flushPixels();
	}
}
