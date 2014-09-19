#!/usr/bin/env python

from remote import PIXELS, COLS, ROWS
from itertools import imap
from random import randint
from time import sleep

def run(r):
    for p in PIXELS:
            if randint(0, 2) == 0:
                    r.set_pixel(p, True)
    r.flush_pixels()

    for _ in xrange(100):
        sleep(0.2)
        queue = []
        for p in PIXELS:
            x, y = p
            living = sum(imap(r.get_pixel, (
                (((x - 1) % COLS, y)),
                (((x + 1) % COLS, y)),
                (((x - 1) % COLS, (y - 1) % ROWS)),
                (((x + 1) % COLS, (y + 1) % ROWS)),
                (((x - 1) % COLS, (y + 1) % ROWS)),
                (((x + 1) % COLS, (y - 1) % ROWS)),
                ((x, (y - 1) % ROWS)),
                ((x, (y + 1) % ROWS)),
                )))
            old_state = r.get_pixel(p)
            new_state = (living == 2 and old_state) or living == 3
            if old_state != new_state:
                queue.append((p, new_state))
        for item in queue:
            r.set_pixel(*item)
        r.flush_pixels()
