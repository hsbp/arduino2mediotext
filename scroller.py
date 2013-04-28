#!/usr/bin/env python

from __future__ import with_statement
from remote import Remote, PIXELS, ROWS
from time import sleep
from PIL import Image

r = Remote()
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
