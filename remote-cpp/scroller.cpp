#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>

#include "Remote.h"
#include "runner.h"

void run(Remote &r) {
	FILE *f = fopen("hack-scroll.pbm", "rb");
	fseek(f, 0, SEEK_END);
	const long length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char pbm[length];
	const bool pbmRead = fread(pbm, length, 1, f);
	fclose(f);

	if (!pbmRead) return;

	regex_t preg;
	regmatch_t matches[3];
	if (regcomp(&preg, "^P4[[:space:]]+([0-9]+)[[:space:]]+([0-9]+)[[:space:]]", REG_EXTENDED)) return;
	if (regexec(&preg, pbm, 3, matches, 0)) return;
	if (matches[2].rm_so < 0) return;
	regfree(&preg);

	pbm[matches[1].rm_eo] = 0;
	const int width = atoi(pbm + matches[1].rm_so);
	pbm[matches[2].rm_eo] = 0;
	const int height = atoi(pbm + matches[2].rm_so);

	char *pixelBuf = pbm + matches[2].rm_eo + 1;
	const int pixelLen = length - matches[2].rm_eo - 1;
	if (pixelLen < width * height / 8) return;

	const uint16_t rowSize = (width - 1) / 8 + 1;
	
	for (int offset = 0; offset < width; offset++) {
		usleep(30000);
		for (uint8_t y = 0; y < ROWS; y++) {
			const uint16_t rowOffset = rowSize * y;
			for (uint8_t x = 0; x < COLS; x++) {
				const uint16_t px = x + offset;
				if (px >= width) continue;
				r.setPixel(x, y, (pixelBuf[rowOffset + px / 8] & (1 << (7 - (px % 8)))) == 0);
			}
		}
		r.flushPixels();
	}
}
