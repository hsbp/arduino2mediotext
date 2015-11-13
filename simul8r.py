#!/usr/bin/env python

from __future__ import with_statement, print_function
from remote import COLS, ROWS
from blessings import Terminal
from SocketServer import UnixStreamServer, StreamRequestHandler

SOCK_NAME = 'simul8r.sock'

class Simul8r(object):
    def __init__(self, **kwargs):
        self.term = Terminal()
        self.writebuf = ''
        if self.term.width < COLS:
            raise RuntimeError("Terminal must be at least {0} characters wide".format(COLS))
        if self.term.height < ROWS:
            raise RuntimeError("Terminal must be at least {0} characters high".format(ROWS))
        self.framebuf = [None] * COLS * ROWS

    def repaint(self, clip):
        (sx, sy), (ex, ey) = clip
        fb = self.framebuf
        for y in xrange(sy, ey):
            with self.term.location(sx, y):
                print(''.join(self.pixel2chr(pixel) for pixel in fb[y * COLS + sx:y * COLS + ex]))

    def write(self, data):
        if (ord(data[0]) & 0xc0) == 0xc0:
            x1, y1, x2, y2 = parse_header(data)
            width = (x2 - x1) * 8
            for cn, char in enumerate(data[2:]):
                bits = '{0:08b}'.format(ord(char))
                for bn, bit in enumerate(bits):
                    pn = cn * 8 + bn
                    x = x1 * 8 + (pn % width)
                    y = y1 + (pn / width)
                    self.framebuf[y * COLS + x] = (bit == '1')
            clip = (x1 * 8, y1), (x2 * 8, y2)
            self.writebuf += chr(0x42)
            self.repaint(clip)

    def read(self):
        retval = self.writebuf
        self.writebuf = ''
        return retval

    def pixel2chr(self, pixel):
        return self.term.red_bold('o') if pixel else ' '

class Simul8rHandler(StreamRequestHandler):
    def handle(self):
        sim = Simul8r()
        while True:
            header = self.request.recv(2)
            if len(header) != 2:
                break
            x1, y1, x2, y2 = parse_header(header)
            width = x2 - x1
            height = y2 - y1
            payload = self.request.recv(width * height)
            sim.write(header + payload)
            self.request.sendall(sim.read())

def parse_header(data):
    first = ord(data[0])
    y1 = (first >> 3) & 0x07
    y2 = first & 0x07
    second = ord(data[1])
    x1 = second >> 4
    x2 = second & 0x0F
    return x1, y1, x2, y2

if __name__ == '__main__':
    try:
        UnixStreamServer(SOCK_NAME, Simul8rHandler).handle_request()
    finally:
        from os import remove
        remove(SOCK_NAME)
