#!/usr/bin/env python

from serial import Serial
from time import sleep
from itertools import product, imap
from operator import itemgetter

COLS = 96
BIT_PER_BYTE = 8
ROWS = 7
PIXELS = set(product(xrange(COLS), xrange(ROWS)))
GET_X = itemgetter(0)
GET_Y = itemgetter(1)

class Remote(object):
    def __init__(self, serial_class=Serial):
        self.port = serial_class('/dev/ttyUSB0', 19200, timeout=0.1)
        self.framebuf = [None] * ROWS * COLS
        sleep(2)
        self.clip = set()
        for pixel in PIXELS:
			self.set_pixel(pixel, False)
        self.flush_pixels()

    def write_pixeldata(self, b1, b2, block_pixels):
        assert valid_block(b1)
        assert valid_block(b2)
        x1, y1 = b1
        x2, y2 = b2
        assert (x2 - x1) * (y2 - y1) == len(block_pixels)
        raw = ''.join((
            chr(0xC0 | (y1 << 3) | y2),
            chr((x1 << 4) | x2),
            block_pixels))
        self.port.write(raw)
        while not self.port.read():
            pass

    def get_pixel(self, pixel):
        return self.framebuf[pixel2fbindex(pixel)]

    def set_pixel(self, pixel, value):
        self.framebuf[pixel2fbindex(pixel)] = value
        block = pixel2block(pixel)
        self.clip.add(block)

    def flush_pixels(self):
        clip = self.clip
        if not clip:
            return
        xs = set(imap(GET_X, clip))
        ys = set(imap(GET_Y, clip))
        left = min(xs)
        right = max(xs) + 1
        top = min(ys)
        bottom = max(ys) + 1
        width = right - left
        pixeldata = ''.join(
                self.get_pixeldata_byte((left + offset % width, top + offset / width))
                for offset in xrange((bottom - top) * width))
        self.write_pixeldata((left, top), (right, bottom), pixeldata)
        self.clip = set()

    def get_pixeldata_byte(self, block):
        retval = 0
        bx, by = block
        index = pixel2fbindex((bx * BIT_PER_BYTE, by))
        for pos, bit in enumerate(self.framebuf[index:index + 8]):
            if bit:
                retval |= 0x80 >> pos
        return chr(retval)

def pixel2fbindex(pixel):
    col, row = pixel
    return row * COLS + col

def pixel2block(pixel):
    col, row = pixel
    return col / BIT_PER_BYTE, row

def valid_block(block):
    x, y = block
    return 0 <= x <= (COLS / BIT_PER_BYTE) and 0 <= y <= ROWS

def next_block(block):
    x, y = block
    return x + 1, y + 1

if __name__ == '__main__':
    r = Remote()
    for y in xrange(ROWS):
        for x in xrange(COLS):
            r.set_pixel((x, y), True)
            r.flush_pixels()
            sleep(0.05)
            r.set_pixel((x, y), False)
