#!/usr/bin/env python

# basic test architectures. Must be written in a test_*.py file

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

def test_auth_miss_server():
    serverProc = launch_server('tests/json/proxy.json')
    assert(serverProc != None)
    requ = custom_request('tests/custom/auth_miss_server.txt', 'sub.localhost', 8080)
    assert(requ.status == 401)
    kill_server(serverProc)

def test_auth_wrong_server():
    serverProc = launch_server('tests/json/proxy.json')
    assert(serverProc != None)
    requ = requests.get('http://127.0.1.7:8080/', auth=('lucas','yolo'))
    assert(requ.status_code == 401)
    kill_server(serverProc)

def test_auth_valid_server():
    serverProc = launch_server('tests/json/proxy.json')
    assert(serverProc != None)
    requ = requests.get('http://127.0.1.7:8080/', auth=('lucas','bestspider'))
    assert(requ.status_code == 200)
    kill_server(serverProc)

def test_auth_miss_proxy():
    serverProc = launch_server('tests/json/proxy.json')
    assert(serverProc != None)
    requ = custom_request('tests/custom/auth_miss_proxy.txt', 'localhost', 8081)
    assert(requ.status == 407)
    kill_server(serverProc)

def test_auth_wrong_proxy():
    serverProc = launch_server('tests/json/proxy.json')
    assert(serverProc != None)
    requ = requests.get('http://127.0.0.1:8081/', auth=('lucas','yolo'))
    assert(requ.status_code == 407)
    kill_server(serverProc)
