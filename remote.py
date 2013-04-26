#!/usr/bin/env python

from serial import Serial
from time import sleep

COLS = 96
BIT_PER_BYTE = 8
ROWS = 7

class Remote(object):
    def __init__(self):
        self.port = Serial('/dev/ttyUSB0', 19200, timeout=0.1)
        self.framebuf = [None] * ROWS * COLS
        sleep(2)
        self.clip = None
        for row in xrange(ROWS):
            for col in xrange(COLS):
                self.set_pixel((col, row), False)
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
        if self.clip is None:
            self.clip = block, next_block(block)
        else:
            (left, top), (right, bottom) = self.clip
            x, y = block
            left = min(left, x)
            top = min(top, y)
            right = max(right, x + 1)
            bottom = max(bottom, y + 1)
            self.clip = (left, top), (right, bottom)

    def flush_pixels(self):
        if self.clip is None:
            return
        (left, top), (right, bottom) = self.clip
        width = right - left
        pixeldata = ''.join(
                self.get_pixeldata_byte((left + offset % width, top + offset / width))
                for offset in xrange((bottom - top) * width))
        self.write_pixeldata((left, top), (right, bottom), pixeldata)
        self.clip = None

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
