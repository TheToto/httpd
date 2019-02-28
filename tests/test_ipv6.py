#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
from spider_misc import *


def test_ipv6():
    serverProc = launch_server("tests/json/ipv6.json")
    assert(serverProc != None)

    requ_ok = requests.get('http://[::1]:8000/tests/index.html')
    requ_head = requests.head('http://[::1]:8000/tests/index.html')
    requ_404 = requests.get('http://[::1]:8000/tests/doesnotexist')
    requ_post = requests.post('http://[::1]:8000/tests/')
    requ_method = requests.put('http://[::1]:8000/puuut')

    assert(requ_ok.status_code == 200)
    assert(requ_head.status_code == 200)
    assert(requ_404.status_code == 404)
    assert(requ_post.status_code == 200)
    assert(requ_method.status_code == 405)  # Method not Allowed

    body = open("tests/index.html", "r").read()
    assert(requ_ok.text == body)
    assert(requ_post.text == body)

    assert(requ_head.headers['content-length'] != "0")
    assert(requ_head.text != body)

    os.killpg(os.getpgid(serverProc.pid), signal.SIGINT)
