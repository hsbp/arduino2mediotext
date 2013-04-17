#!/usr/bin/env python

from serial import Serial
from time import sleep
from datetime import datetime

class Remote(object):
    def __init__(self):
        self.port = Serial('/dev/ttyUSB0', 19200, timeout=0.1)

    def write_pixeldata(self, b1, b2, block_pixels):
        assert valid_block(b1)
        assert valid_block(b2)
        x1, y1 = b1
        x2, y2 = b2
        assert (x2 - x1) * (y2 - y1) == len(block_pixels)
        self.port.write(''.join((
            chr(0xC0 | (y1 << 3) | y2),
            chr((x1 << 4) | x2),
            block_pixels)))
        while not self.port.read():
            pass

def valid_block(block):
    x, y = block
    return 0 <= x <= 12 and 0 <= y <= 7

if __name__ == '__main__':
    r = Remote()
    sleep(1)
    for i in xrange(100):
        dt = datetime.now()
        r.write_pixeldata((0, 0), (12, 7), '\x55' * 12 * 7)
        print datetime.now() - dt
        r.write_pixeldata((0, 0), (12, 7), '\xAA' * 12 * 7)
