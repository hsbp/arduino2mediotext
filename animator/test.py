#!/usr/bin/env python

from __future__ import with_statement
from binascii import unhexlify
import struct

ROWS = 96
COLS = 7
BITS_PER_BYTE = 8

FRAMES = ROWS * COLS
WAIT = 20

with file('test.bin', 'wb') as f:
    f.write(struct.pack('>HH', FRAMES, WAIT));
    fmt = '{0:0' + str(FRAMES / 4) + 'x}'
    for frame_no in xrange(FRAMES):
        f.write(unhexlify(fmt.format(1 << (FRAMES - frame_no - 1))))
