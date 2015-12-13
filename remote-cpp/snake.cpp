#include <SDL2/SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "Remote.h"

#define LEFT  0x00FF
#define RIGHT 0x0001
#define UP    0x08FF
#define DOWN  0x0801
#define INVALID_DIR 0

#define OPPOSITE(D) (((D) & 0xFF00) | (0x100 - ((D) & 0xFF)))
#define PIXELS_SIZE (COLS * ROWS)
#define ENCODE_PIXEL(X, Y) (((Y) << 8) | (X))
#define DECODE_PIXEL_X(P) ((P) & 0xFF)
#define DECODE_PIXEL_Y(P) ((P) >> 8)

#define AXIS_TO_DIR(A) ((A) * 0x800)
#define VALUE_TO_DIR(V) ((V) < 0 ? 0xFF : 0x01)

class Dot {

public:
	Dot(Remote *r);
	void draw(bool value);
	bool posEquals(uint16_t pos);

protected:
	uint8_t m_x, m_y;
	Remote *m_remote;
};

Dot::Dot(Remote *r) {
	do {
		m_x = rand() % COLS;
		m_y = rand() % ROWS;
	} while (r->getPixel(m_x, m_y));
	m_remote = r;
	draw(true);
}

void Dot::draw(bool value) {
	m_remote->setPixel(m_x, m_y, value);
}

bool Dot::posEquals(uint16_t pos) {
	return ENCODE_PIXEL(m_x, m_y) == pos;
}

class Snake {

public:
	Snake(Remote *r, uint16_t direction);
	void grow();
	uint16_t step();
	void draw(bool value);
	void turn(uint16_t direction);

protected:
	uint16_t m_direction;
	uint8_t m_needGrowth;
	uint16_t m_pixelsBuf[PIXELS_SIZE];
	uint16_t m_pixelsStart;
	uint16_t m_pixelsLength;
	Remote *m_remote;
};

Snake::Snake(Remote *r, uint16_t direction) {
	m_remote = r;
	m_direction = direction;
	m_needGrowth = 6;
	m_pixelsBuf[0] = ENCODE_PIXEL(10, 0);
	m_pixelsStart = 0;
	m_pixelsLength = 1;
	draw(true);
}

void Snake::grow() {
	m_needGrowth++;
}

uint16_t Snake::step() {
	const uint16_t headPixel = m_pixelsBuf[(m_pixelsStart + m_pixelsLength - 1) % PIXELS_SIZE];
	const uint8_t shift = m_direction >> 8;
	const uint8_t nextValue = ((headPixel >> shift) & 0xFF) + ((int8_t)(m_direction & 0xFF));
	const uint16_t nextPixel = (headPixel & (0xFF << (8 - shift))) | (nextValue << shift);

	const uint8_t nextX = DECODE_PIXEL_X(nextPixel), nextY = DECODE_PIXEL_Y(nextPixel);

	if (nextX >= COLS || nextY >= ROWS || m_remote->getPixel(nextX, nextY)) {
		throw this;
	}

	m_pixelsBuf[(m_pixelsStart + m_pixelsLength) % PIXELS_SIZE] = nextPixel;
	
	m_remote->setPixel(nextX, nextY, true);

	if (m_needGrowth) {
		m_needGrowth--;
		m_pixelsLength++;
	} else {
		const uint16_t tail = m_pixelsBuf[m_pixelsStart];
		m_remote->setPixel(DECODE_PIXEL_X(tail), DECODE_PIXEL_Y(tail), false);
		if (m_pixelsStart == PIXELS_SIZE - 1) m_pixelsStart = 0; else m_pixelsStart++;
	}

	return ENCODE_PIXEL(nextX, nextY);
}

void Snake::draw(bool value) {
	for (uint16_t pi = 0; pi < m_pixelsLength; pi++) {
		const uint16_t pixel = m_pixelsBuf[(m_pixelsStart + pi) % PIXELS_SIZE];
		m_remote->setPixel(DECODE_PIXEL_X(pixel), DECODE_PIXEL_Y(pixel), value);
	}
}

void Snake::turn(uint16_t direction) {
	if (OPPOSITE(direction) == m_direction || direction == m_direction) return;
	m_direction = direction;
}

void run(Remote &r) {
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return;
	}
	if (SDL_NumJoysticks() < 1) {
		printf("No joysticks found, check permissions.\n");
		return;
	}

	srand(time(NULL));

	SDL_JoystickEventState(SDL_ENABLE);
	SDL_Joystick *joystick = SDL_JoystickOpen(0);

	while (true) {
		Snake snake(&r, RIGHT);

		try
		{
			while (true) {
				Dot dot(&r);

				while (true) {
					r.flushPixels();
					for (uint8_t i = 0; i < 3; i++) {
						usleep(30000);
						dot.draw((i % 2) == 1);
						r.flushPixels();
					}

					const uint16_t next = snake.step();
					if (dot.posEquals(next)) {
						snake.grow();
						break;
					}

					SDL_Event event;

					uint16_t nextTurn = INVALID_DIR;

					while (SDL_PollEvent(&event)) {
						switch (event.type) {
							case SDL_JOYAXISMOTION:
								{
									const int value = event.jaxis.value;
									if (value < -3200 || value > 3200) {
										nextTurn = AXIS_TO_DIR(event.jaxis.axis) |
											VALUE_TO_DIR(value);
									}
								}
								break;
							case SDL_QUIT:
								return;
						}
					}

					if (nextTurn != INVALID_DIR) snake.turn(nextTurn);
				}
			}
		} catch (Snake *crash) {
			for (uint8_t i = 0; i < 32; i++) {
				snake.draw((i % 2) == 1);
				usleep(50000);
				r.flushPixels();
			}
			snake.draw(false);
		}
	}
}
