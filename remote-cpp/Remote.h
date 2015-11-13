#ifndef REMOTE_H
#define REMOTE_H

#include <stdint.h>

#define COLS 96
#define BIT_PER_BYTE 8
#define ROWS 7

class Remote {

protected:
	uint8_t m_framebuf[COLS * ROWS / BIT_PER_BYTE];
	uint8_t m_clip[ROWS * COLS / BIT_PER_BYTE / BIT_PER_BYTE + 1];
	int m_fdrd, m_fdwr;

public:
	Remote(int fdrd, int fdwr);
	bool getPixel(uint8_t x, uint8_t y) const;
	void setPixel(uint8_t x, uint8_t y, bool value);
	void flushPixels();
};

#endif
