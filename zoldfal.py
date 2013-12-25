#!/usr/bin/env python

from remote import ROWS, COLS, PIXELS
from PIL import Image, ImageFont, ImageDraw
from time import sleep
from flask import Flask, request
import json

app = Flask(__name__)
font = ImageFont.load_default()
bg = Image.open('zoldfal.png')
drawn = False
schema = (('t1', 'TEMP1'), ('t2', 'TEMP2'))

def display(r, a, b, n, t, offset=0):
    global drawn
    img = Image.new('1', (COLS, ROWS))
    draw = ImageDraw.Draw(img)
    img.paste(bg, (0, 0))
    left = 64
    def dt((x, y), text):
        for n, row in enumerate(text.split('\n')):
            draw.text((x, y + 8 * n), row, 1, font=font)
    dt((26, offset - 2), n)
    dt((left, offset - 2), a)
    dt((left + 15, offset - 2), b)
    for pixel in PIXELS:
        if not drawn or 26 <= pixel[0] <= 85:
            r.set_pixel(pixel, img.getpixel(pixel))
    r.flush_pixels()
    drawn = True
    sleep(t)

def run(r):
    with file('zoldfal.json', 'rb') as db:
        payload = json.load(db)
    ts = [(title, payload[key]) for key, title in schema]
    prev_n = None
    prev_t = None
    for n, t in ts:
        if prev_n is not None:
            a = '{0:2}\n{1:2}'.format(int(t), int(prev_t))
            b = str((t * 10) % 10)[0] + '\n' + str((prev_t * 10) % 10)[0]
            for i in xrange(7):
                display(r, a, b, n + '\n' + prev_n, 0.15, i - 7)
        a = '{0:2}'.format(int(t))
        b = str((t * 10) % 10)[0]
        display(r, a, b, n, 5)
        prev_n = n
        prev_t = t

@app.route('/save')
def save():
    to_save = {key: float(request.args[key]) for key, _title in schema}
    with file('zoldfal.json', 'wb') as db:
        json.dump(to_save, db)
    return ''

if __name__ == '__main__':
    app.run(debug=True)
