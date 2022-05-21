from ctypes import *
import unittest


mimelib = CDLL("tests/mime.so")

class HTTPtest(unittest.TestCase):
    def test_is_file_script01(self):
        ext = c_char_p(b'php')
        self.assertEqual(mimelib.is_file_script(ext), 0)
    
    def test_is_file_script02(self):
        ext = c_char_p(b'py')
        self.assertEqual(mimelib.is_file_script(ext), 0)

    def test_is_file_script03(self):
        ext = c_char_p(b'phpy')
        self.assertEqual(mimelib.is_file_script(ext), 1)

if __name__ == '__main__':
    unittest.main()
