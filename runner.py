#!/usr/bin/env python

from argparse import ArgumentParser
from remote import Remote

def main():
    parser = ArgumentParser(description='Runs Mediotext scripts')
    parser.add_argument('module', help='Python module name')
    parser.add_argument('--sport', dest='sport', default='/dev/ttyUSB0',
            help='Serial port (default: /dev/ttyUSB0)')
    parser.add_argument('--drawille', dest='drawille', action='store_true',
            help='Use drawille to simulate display')
    parser.add_argument('--host', dest='host',
            help='Remote host to connect to')
    parser.add_argument('--tport', dest='tport', default=42493,
            help='Remote port to connect to (default: 42493)')
    args = parser.parse_args()
    if args.drawille:
        from drawille_bridge import DrawilleBridge
        r = DrawilleBridge()
    elif args.host:
        from tcp import TcpSerialBridge
        r = Remote(serial_class=TcpSerialBridge((args.host, args.tport)))
    else:
        r = Remote(port=args.sport)
    __import__(args.module).run(r)


if __name__ == '__main__':
    main()
