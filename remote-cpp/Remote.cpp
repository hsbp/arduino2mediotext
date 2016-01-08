#include "Remote.h"
#include <string.h>
#include <unistd.h>

#define INVALID_CLIP 0xFF

#define PIXEL_TO_FB_INDEX(x, y) (((y) * COLS + x) / BIT_PER_BYTE)
#define PIXEL_TO_FB_BIT(x) (1 << (7 - (x % BIT_PER_BYTE)))

#define FB_INDEX_TO_CLIP_INDEX(fbi) ((fbi) / BIT_PER_BYTE)
#define FB_INDEX_TO_CLIP_BIT(x) PIXEL_TO_FB_BIT(x)

Remote::Remote(int fdrd, int fdwr) {
	m_fdrd = fdrd;
	m_fdwr = fdwr;
	for (uint8_t y = 0; y < ROWS; y++) {
		for (uint8_t x = 0; x < COLS; x++) {
			setPixel(x, y, false);
		}
	}
	flushPixels();
}

/*
 * framebuf:
 *
 *     x=0      x=8          x=88
 * y=0 00000000 00000000 ... 00000000
 * y=1 00000000 00000000 ... 00000000
 *     ...      ...      ... ...
 * y=6 00000000 00000000 ... 00000000
 *
 * clip:
 *
 *     y=0          y=1           y=2  
 *     x=0      x=64x=0      x=64 x=0
 *     00000000 00000000 00000000 00000000 ...
 */ 

void Remote::flushPixels() {
	uint8_t left = INVALID_CLIP, right = INVALID_CLIP, top = INVALID_CLIP, bottom = INVALID_CLIP;
	uint8_t idx = 0, bit = 0x80;
	uint8_t clip = m_clip[0];
	for (uint8_t y = 0; y < ROWS; y++) { 
		for (uint8_t x = 0; x < COLS / BIT_PER_BYTE; x++) {
			if (clip & bit) {
				if (top == INVALID_CLIP || top > y) top = y;
				if (bottom == INVALID_CLIP || bottom <= y) bottom = y + 1;
				if (left == INVALID_CLIP || left > x) left = x;
				if (right == INVALID_CLIP || right <= x) right = x + 1;

			}
			if (bit == 1) {
				bit = 0x80;
				clip = m_clip[++idx];
			} else bit >>= 1;
		}
	}
	if (left == INVALID_CLIP) return;

	const uint8_t width = right - left;
	const uint8_t height = bottom - top;
	const size_t buflen = 2 + height * width;
	uint8_t buf[buflen];

	buf[0] = 0xC0 | (top << 3) | bottom;
	buf[1] = (left << 4) | right;
	for (uint8_t y = 0; y < height; y++) {
		memcpy(buf + 2 + y * width, m_framebuf + left + (y + top) * COLS / BIT_PER_BYTE, width);
	}

	write(m_fdwr, buf, buflen);
	while (!read(m_fdrd, buf, 1));

	memset(m_clip, 0x00, sizeof(m_clip));
}

void Remote::setPixel(uint8_t x, uint8_t y, bool value) {
	const uint8_t idx = PIXEL_TO_FB_INDEX(x, y);
	const uint8_t old = m_framebuf[idx];
	const uint8_t mask = PIXEL_TO_FB_BIT(x);
	if (value) {
		if (mask & old) return;
		m_framebuf[idx] = old | mask;
	} else {
		if ((mask & old) == 0) return;
		m_framebuf[idx] = old & ~mask;
	}
	m_clip[FB_INDEX_TO_CLIP_INDEX(idx)] |= FB_INDEX_TO_CLIP_BIT(idx);
}

bool Remote::getPixel(uint8_t x, uint8_t y) const {
	return (m_framebuf[PIXEL_TO_FB_INDEX(x, y)] & PIXEL_TO_FB_BIT(x)) != 0;
}
