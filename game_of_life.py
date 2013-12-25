#!/usr/bin/env python

from remote import PIXELS, COLS, ROWS, pixel2fbindex
from random import randint
from time import sleep

def run(r):
    for p in PIXELS:
            if randint(0, 2) == 0:
                    r.set_pixel(p, True)
    r.flush_pixels()

    i = 0
    ps = list(PIXELS)

    for _ in xrange(100):
            sleep(0.2)
            fb_old = list(r.framebuf)
            for p in PIXELS:
                    x, y = p
                    living = sum((
                            fb_old[pixel2fbindex(((x - 1) % COLS, y))],
                            fb_old[pixel2fbindex(((x + 1) % COLS, y))],
                            fb_old[pixel2fbindex(((x - 1) % COLS, (y - 1) % ROWS))],
                            fb_old[pixel2fbindex(((x + 1) % COLS, (y + 1) % ROWS))],
                            fb_old[pixel2fbindex(((x - 1) % COLS, (y + 1) % ROWS))],
                            fb_old[pixel2fbindex(((x + 1) % COLS, (y - 1) % ROWS))],
                            fb_old[pixel2fbindex((x, (y - 1) % ROWS))],
                            fb_old[pixel2fbindex((x, (y + 1) % ROWS))],
                            ))
                    old_state = fb_old[pixel2fbindex(p)]
                    new_state = (living == 2 and old_state) or living == 3
                    if old_state != new_state:
                            r.set_pixel(p, new_state)
            r.flush_pixels()
