#!/usr/bin/env python

from __future__ import print_function
from drawille import Canvas
from blessings import Terminal
from remote import COLS, ROWS

class DrawilleBridge(object):
    def __init__(self, *args, **kwargs):
        self.term = Terminal()
        self.canvas = Canvas()

    def get_pixel(self, pixel):
        return self.canvas.get(*pixel)

    def set_pixel(self, pixel, value):
        setter = self.canvas.set if value else self.canvas.unset
        setter(*pixel)

    def flush_pixels(self):
        with self.term.location(0, 0):
            print(self.canvas.frame(min_x=0, min_y=0, max_x=COLS, max_y=ROWS))
