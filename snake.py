#!/usr/bin/env python

from remote import Remote, PIXELS, COLS
from random import choice
from time import sleep
from itertools import izip
from PIL import Image
from threading import Thread, RLock
import pygame, sys

try:
	num_snakes = int(sys.argv[1])
	assert 1 <= num_snakes <= 2
except (IndexError, ValueError, AssertionError):
	print >>sys.stderr, 'Usage: {0} <number of players>'.format(sys.argv[0])
	raise SystemExit(1)

WALL = (0, 0, 0)
START = (255, 0, 0)
END = (255, 255, 255)

levels = []
level_num = 1
while True:
	try:
		level = Image.open("LAB{0}.png".format(level_num)).convert('RGB').load()
		tmp = {WALL: [], START: [], END: []}
		for pixel in PIXELS:
			p = level[pixel]
			for k, v in tmp.iteritems():
				if p == k:
					v.append(pixel)
					break
		levels.append(tmp)
		level_num += 1
	except:
		break

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
			0: ((10, 0), DOWN),
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

	def __init__(self, level):
		while self.position is None:
			pos_candidate = choice(level[END])
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
		new_dir = Snake.OPPOSITES[new_dir] # 180 degrees flip
		if snake.direction not in (Snake.OPPOSITES[new_dir], new_dir):
			snake.direction = new_dir

joystick = Joystick()
level_num = 0

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
	level = levels[level_num]
	for p in ps:
		r.set_pixel(p, False)
	for wall in levels[level_num][WALL]:
		r.set_pixel(wall, True)
	snakes = [Snake(i) for i in xrange(num_snakes)]
	joystick.flush()
	for snake in snakes:
		snake.draw()
	dot = Dot(level)
	r.flush_pixels()
	try:
		while True:
			for i in xrange(3):
				sleep(0.03 * 2)
				dot.draw((i % 2) == 1)
				r.flush_pixels()
			joystick.get_dir(snakes)
			for snake in snakes:
				if dot.position in snake.pixels:
					level_num += 1
					if level_num == len(levels):
						level_num = 0
					raise SnakeCrash(snake)
					snake.grow()
					dot = Dot(level)
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
