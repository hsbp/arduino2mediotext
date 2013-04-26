#!/usr/bin/env python

from StringIO import StringIO
import unittest
import remote


class TestRemote(unittest.TestCase):
    def setUp(self):
        self.remote = remote.Remote()

    def test_init(self):
        self.assertEqual(
                self.get_serial_buf().getvalue(),
                '\xC7\x0C' + '\x00' *
                    (remote.COLS / remote.BIT_PER_BYTE * remote.ROWS))
        for x in xrange(remote.COLS):
            for y in xrange(remote.ROWS):
                self.assertFalse(self.remote.get_pixel((x, y)))

    def test_first_block(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        self.remote.set_pixel((0, 0), True)
        self.remote.flush_pixels()
        self.assertEqual(sb.getvalue(), '\xC1\x01\x80')

    def test_second_block(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        self.remote.set_pixel((8, 0), True)
        self.remote.flush_pixels()
        self.assertEqual(sb.getvalue(), '\xC1\x12\x80')
        sb.truncate(0)
        self.remote.set_pixel((9, 0), True)
        self.remote.flush_pixels()
        self.assertEqual(sb.getvalue(), '\xC1\x12\xc0')

    def test_empty_flush(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        self.remote.flush_pixels()
        self.assertEquals(sb.getvalue(), '')
		
    def test_full(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        for x in xrange(remote.COLS):
            for y in xrange(remote.ROWS):
                self.remote.set_pixel((x, y), True)
        for x in xrange(remote.COLS):
            for y in xrange(remote.ROWS):
                self.assertTrue(self.remote.get_pixel((x, y)))
        self.remote.flush_pixels()
        self.assertEqual(
                sb.getvalue(),
                '\xC7\x0C' + '\xFF' *
                    (remote.COLS / remote.BIT_PER_BYTE * remote.ROWS))

    def get_serial_buf(self):
        return self.remote.port.dut2tc


class MockSerial(object):
    def __init__(self, port, speed, timeout):
        self.dut2tc = StringIO()
        self.tried_read = False

    def write(self, data):
        pp = self.dut2tc.tell()
        self.dut2tc.write(data)
        self.dut2tc.seek(pp)

    def read(self):
        if self.tried_read:
            self.tried_read = False
            return 'B'
        else:
            self.tried_read = True
            return ''


remote.Serial = MockSerial

if __name__ == '__main__':
    unittest.main()
