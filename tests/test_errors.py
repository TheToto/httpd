#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
from spider_misc import *

def test_404():
    serverProc = launch_server()
    assert(serverProc != None)


    requ1 = requests.get('http://127.0.0.1:8000/tests/doesnotexist')
    requ2 = requests.head('http://127.0.0.1:8000/tests/doesnotexist')
    requ3 = requests.post('http://127.0.0.1:8000/tests/doesnotexist')
    requ4 = requests.put('http://127.0.0.1:8000/tests/doesnotexist')

    assert(requ1.status_code == 404)
    assert(requ2.status_code == 404)
    assert(requ3.status_code == 404)

    assert(requ2.headers['content-length'] == "0")

    assert(requ4.status_code == 405)  # Method not Allowed

    os.killpg(os.getpgid(serverProc.pid), signal.SIGINT)

def test_403():
    serverProc = launch_server()
    assert(serverProc != None)

    if os.path.isfile("tests/nope"):
        os.chmod("tests/nope", 644)
    open("tests/nope", 'a').close()
    os.chmod("tests/nope", 000)
    requ1 = requests.get('http://127.0.0.1:8000/tests/nope')
    requ2 = requests.head('http://127.0.0.1:8000/tests/nope')
    requ3 = requests.post('http://127.0.0.1:8000/tests/nope')
    requ4 = requests.put('http://127.0.0.1:8000/tests/nope')

    assert(requ1.status_code == 403)
    assert(requ2.status_code == 403)
    assert(requ3.status_code == 403)

    assert(requ2.headers['content-length'] == "0")

    assert(requ4.status_code == 405)  # Method not Allowed

    os.chmod("tests/nope", 644)
    os.remove("tests/nope")
    kill_server(serverProc)