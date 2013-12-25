#!/usr/bin/env python

from remote import ROWS, COLS, PIXELS
from PIL import Image, ImageFont, ImageDraw
from datetime import datetime
from time import sleep
import requests

ISO_DATETIME_FMT = '%Y-%m-%d %H:%M:%S'
STATIC_COL = 45

status_json = requests.get('http://vsza.hu/hacksense/status.json').json()
since = datetime.strptime(status_json['when'], ISO_DATETIME_FMT)

def run(r):
    lock = Image.open('lock.png')
    img = Image.new('1', (COLS, ROWS))
    font = ImageFont.load_default()
    draw = ImageDraw.Draw(img)
    for i in xrange(11):
        img.paste(0, (STATIC_COL, 0, COLS, ROWS))
        if (i % 2) == 0:
            img.paste(lock, (0, 0))
        dt = (datetime.now() - since).seconds
        delta = '{0:02d} {1:02d} {2:02d}'.format(
                dt / 3600,
                (dt / 60) % 60,
                dt % 60
                )
        draw.text((48, -2), delta, 1, font=font)
        for pixel in PIXELS:
            if i == 0 or pixel[0] >= STATIC_COL:
                r.set_pixel(pixel, img.getpixel(pixel))
        r.flush_pixels()
        sleep(1)
