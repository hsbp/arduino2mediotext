#!/usr/bin/env python

# src: http://www.stuartaxon.com/2010/05/17/shoebot-spectrograph/

from remote import ROWS, COLS
from numpy import short, fromstring
from numpy.fft import fft
from math import log10
import pyaudio

NUM_SAMPLES = COLS * 2
SAMPLING_RATE = 11025

_stream = None
pa = None

def read_fft():
    global _stream, pa

    if _stream is None:
        pa = pyaudio.PyAudio()
        _stream = pa.open(format=pyaudio.paInt16, channels=1,
                           rate=SAMPLING_RATE,
                           input=True, frames_per_buffer=NUM_SAMPLES)

    audio_data  = fromstring(_stream.read(NUM_SAMPLES), dtype=short)
    normalized_data = audio_data / 32768.0

    return fft(normalized_data)[1:1+NUM_SAMPLES/2]

def flatten_fft(scale = 1.0):
    """
    Produces a nicer graph, I'm not sure if this is correct
    """
    for i, v in enumerate(read_fft()):
        yield scale * (i * v) / NUM_SAMPLES

def run(r):
    global _stream
    for _ in xrange(500):
        for x, s in enumerate(flatten_fft(scale = 80)):
            level = int(round((1 - log10(0.0001 + abs(s))) * ROWS))
            for y in xrange(ROWS):
                r.set_pixel((x, y), y > level)
        r.flush_pixels()
        _stream.read(NUM_SAMPLES * 3) # XXX WorksHere(TM)
    if _stream:
        _stream.stop_stream()
        _stream.close()
    pa.terminate()
