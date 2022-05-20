import http
from concurrent.futures import ThreadPoolExecutor
from threading import Thread
import unittest
import requests
import http.client

DFLT_IP = 'localhost'
DFLT_PORT = 8080
BASE_URL = f'http://{DFLT_IP}:{DFLT_PORT}'
THREAD_POOL_SIZE = 10

def get_url(url):
    return requests.get(url)

class HTTPtest(unittest.TestCase):
    def test_home_page(self):
        """ test getting basic html page
        """
        r = requests.get(f'{BASE_URL}/home.html')
        self.assertEqual(r.status_code, 200)

    def test_bench_page(self):
        """ test getting benchmarking html page
        """
        r = requests.get(f'{BASE_URL}/benchmark.html')
        self.assertEqual(r.status_code, 200)

    def test_GET(self):
        """ test HTTP GET method
        """
        r = requests.get(f'{BASE_URL}/home.html')
        self.assertEqual(r.status_code, 200)

    def test_POST(self):
        """ test HTTP POST method
        """
        r = requests.post(f'{BASE_URL}/home.html')
        self.assertEqual(r.status_code, 200)

    def test_HEAD(self):
        """ test HTTP HEAD method
        """
        r = requests.head(f'{BASE_URL}/home.html')
        self.assertEqual(r.status_code, 200)

    def test_OPTIONS(self):
        """ test HTTP OPTIONS method
        """
        r = requests.options(f'{BASE_URL}/home.html')
        self.assertEqual(r.status_code, 200)
        self.assertEqual(r.headers['Allow'], str('GET, POST, OPTIONS, HEAD'))

    def test_404_not_found(self):
        """ test HTTP 404 response code
        """
        r = requests.get(f'{BASE_URL}/asdf.htl')
        self.assertEqual(r.status_code, 404)

    def test_501_not_implemented(self):
        """ test HTTP 501 response code by sending wrong method
        """
        httpServ = http.client.HTTPConnection(DFLT_IP, DFLT_PORT)
        httpServ.connect()
        httpServ.request("GETOS", "/home.html")
        r = httpServ.getresponse()
        self.assertEqual(r.status, 501)
    
    def limited_thread_pool(self):
        """ send as many connections as threads are in the pool
        """
        list_of_urls = [f'{BASE_URL}/']*THREAD_POOL_SIZE
        with ThreadPoolExecutor(max_workers=THREAD_POOL_SIZE) as pool:
            response_list = list(pool.map(get_url, list_of_urls))
        
        for response in response_list:
            self.assertEqual(response.status, 200)

    def big_thread_pool(self):
        """ send more connections than threads are in the pool
        """
        list_of_urls = [f'{BASE_URL}/']*(THREAD_POOL_SIZE*100)
        with ThreadPoolExecutor(max_workers=THREAD_POOL_SIZE*100) as pool:
            response_list = list(pool.map(get_url, list_of_urls))
        
        for response in response_list:
            self.assertEqual(response.status, 200)

if __name__ == '__main__':
    unittest.main()