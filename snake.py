#!/usr/bin/env python

from remote import Remote, PIXELS, COLS
from random import choice
from time import sleep
from itertools import izip
from threading import Thread, RLock
import pygame, sys

try:
	num_snakes = int(sys.argv[1])
	assert 1 <= num_snakes <= 2
except (IndexError, ValueError, AssertionError):
	print >>sys.stderr, 'Usage: {0} <number of players>'.format(sys.argv[0])
	raise SystemExit(1)

pygame.init()
j = pygame.joystick.Joystick(0)
j.init()

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
			0: ((6, 0), RIGHT),
			1: ((COLS - 6, 0), LEFT),
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
	def flush(self):
		pass

	def get_dir(self, snakes):
		new_dir = jt.get_dir()
		if not new_dir:
			return
		if snake.direction not in (Snake.OPPOSITES[new_dir], new_dir):
			snake.direction = new_dir

joystick = Joystick()
class JoyThread(Thread):
	new_dir = None

	def __init__(self):
		Thread.__init__(self)
		self.port = pygame.joystick.Joystick(0)
		self.port.init()
		self.lock = RLock()

	def run(self):
		while True:
			sleep(0.001)
			pygame.event.pump()
			x = self.port.get_axis(0)
			y = self.port.get_axis(1)
			with self.lock:
				if x < -0.5:
					self.new_dir = Snake.LEFT
				elif x > 0.5:
					self.new_dir = Snake.RIGHT
				elif y < -0.5:
					self.new_dir = Snake.UP
				elif y > 0.5:
					self.new_dir = Snake.DOWN

	def get_dir(self):
		with self.lock:
			nd = self.new_dir
			self.new_dir = None
		return nd

jt = JoyThread()
jt.start()

while True:
	for p in ps:
		r.set_pixel(p, False)
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
