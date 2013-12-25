#!/usr/bin/env python

import socket

class TcpSerialBridge(object):
    def __init__(self, address=('127.0.0.1', 42493)):
        self.sock = socket.socket()
        self.sock.connect(address)

    def __del__(self):
        self.sock.close()

    def __call__(self, *args, **kwargs):
        return self

    def write(self, data):
        self.sock.sendall(data)

    def read(self):
        return self.sock.recv(1)
