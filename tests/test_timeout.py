#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

def test_keepalive():
    serverProc = launch_server('tests/json/timeout_keepalive.json')
    assert(serverProc != None)

    sock = create_socket()
    make_request(sock, 'tests/custom/valid.txt')
    res = recup_resp(sock)
    assert(res.status == 404)
    time.sleep(3)
    res = recup_resp(sock)
    assert(res.status == 408)


def test_keepalive_without_resquest():
    serverProc = launch_server('tests/json/timeout_keepalive.json')
    assert(serverProc != None)

    sock = create_socket()
    time.sleep(3)
    res = recup_resp(sock)
    assert(res.status == 408)


def test_transaction ():
    serverProc = launch_server('tests/json/timeout_keepalive.json')
    assert(serverProc != None)

    sock = create_socket()
    sock.send(b'blabla')
    time.sleep(3)
    res = recup_resp(sock)
    assert(res.status == 408)

