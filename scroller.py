#!/usr/bin/env python

from remote import PIXELS, ROWS
from time import sleep
from PIL import Image

def run(r):
	img = Image.open('hack-scroll.png')
	width, height = img.size
	assert height == ROWS

	for offset in xrange(width):
		sleep(0.03)
		for dp in PIXELS:
			try:
				x, y = dp
				p = x + offset, y
				pd = img.getpixel(p)
				r.set_pixel(dp, pd)
			except:
				pass
		r.flush_pixels()
