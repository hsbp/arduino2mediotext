#!/usr/bin/env python

import socket

class UnixSerialBridge(object):
    def __init__(self, path='simul8r.sock'):
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(path)

    def __del__(self):
        self.sock.close()

    def __call__(self, *args, **kwargs):
        return self

    def write(self, data):
        self.sock.sendall(data)

    def read(self):
        return self.sock.recv(1)
