#!/usr/bin/env python

import socket

class Recorder(object):
    def __init__(self):
        self.written = []
        self.readable = False

    def __call__(self, *args, **kwargs):
        return self

    def write(self, data):
        self.written.append(data)
        self.readable = True

    def read(self):
        result = chr(0) if self.readable else ''
        self.readable = False
        return result
    
    def save(self, filename):
        with open(filename, 'wb') as f:
            f.write(''.join(self.written))
