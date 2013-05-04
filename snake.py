#!/usr/bin/env python

# for joystick pinout, see https://github.com/dnet/c64joystick-arduino

from serial import Serial
from remote import Remote, PIXELS, COLS, ROWS, pixel2fbindex
from random import randint, choice
from time import sleep
from itertools import izip
import sys

try:
	num_snakes = int(sys.argv[1])
	assert 1 <= num_snakes <= 2
except (IndexError, ValueError, AssertionError):
	print >>sys.stderr, 'Usage: {0} <number of players>'.format(sys.argv[0])
	raise SystemExit(1)

r = Remote()
ps = list(PIXELS)

class SnakeCrash(RuntimeError):
	def __init__(self, snake):
		self.snake = snake

class Snake(object):
	LEFT = (0, -1)
	RIGHT = (0, +1)
	UP = (1, -1)
	DOWN = (1, +1)
	START = {
			0: ((6, 6), RIGHT),
			1: ((90, 6), LEFT),
			}
	OPPOSITES = {LEFT: RIGHT, RIGHT: LEFT, UP: DOWN, DOWN: UP}
	TAIL = 0

	def __init__(self, number):
		p, d = self.START[number]
		self.direction = d
		self.pixels = [p]
		self.need_growth = 6
	
	def draw(self, value=True):
		for pixel in self.pixels:
			r.set_pixel(pixel, value)

	def grow(self):
		self.need_growth += 1

	def step(self):
		next_pixel = list(self.pixels[-1])
		coord, delta = self.direction
		next_pixel[coord] += delta
		next_pixel = tuple(next_pixel)
		if next_pixel not in PIXELS or r.get_pixel(next_pixel):
			raise SnakeCrash(self)
		if self.need_growth:
			self.need_growth -= 1
		else:
			r.set_pixel(self.pixels.pop(self.TAIL), False)
		self.pixels.append(next_pixel)
		self.draw()

class Dot(object):
	position = None

	def __init__(self):
		while self.position is None:
			pos_candidate = choice(ps)
			if not r.get_pixel(pos_candidate):
				self.position = pos_candidate
		self.draw()
	
	def draw(self, value=True):
		r.set_pixel(self.position, value)

class Joystick(object):
	DIR_BITS = [
			[Snake.DOWN, Snake.UP, None, None, None, None, Snake.RIGHT, Snake.LEFT],
			[None, None, Snake.RIGHT, Snake.LEFT, Snake.DOWN, Snake.UP, None, None],
			]

	def __init__(self):
		self.port = Serial('/dev/ttyUSB1', 9600, timeout=0)
	
	def flush(self):
		while self.port.read():
			pass

	def get_dir(self, snakes):
		serial_data = self.port.read()
		num = 0xFF
		if serial_data:
			for input_byte in serial_data:
				num &= ord(input_byte)
		for dir_bits, snake in izip(self.DIR_BITS, snakes):
			for i, new_dir in enumerate(dir_bits):
				if new_dir is None:
					continue
				pwr = 1 << i
				if (num & pwr) == 0 and snake.direction not in (Snake.OPPOSITES[new_dir], new_dir):
					snake.direction = new_dir
					break

joystick = Joystick()

while True:
	snakes = [Snake(i) for i in xrange(num_snakes)]
	joystick.flush()
	for snake in snakes:
		snake.draw()
	dot = Dot()
	r.flush_pixels()
	try:
		while True:
			for i in xrange(3):
				sleep(0.03)
				dot.draw((i % 2) == 1)
				r.flush_pixels()
			joystick.get_dir(snakes)
			for snake in snakes:
				if dot.position in snake.pixels:
					snake.grow()
					dot = Dot()
				snake.step()
			r.flush_pixels()
	except SnakeCrash as sc:
		snake = sc.snake
		for i in xrange(32):
			snake.draw((i % 2) == 1)
			sleep(0.05)
			r.flush_pixels()
	for i, snake in enumerate(snakes):
		print 'Snake {0} was {1} units long'.format(i, len(snake.pixels))
		snake.draw(False)
		dot.draw(False)
	print '------------------------------------------------------'
