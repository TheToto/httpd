#!/usr/bin/env python

import pytest
import requests
import socket
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
    assert(res.status == 200)
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


def test_transaction():
    serverProc = launch_server('tests/json/timeout_trans.json')
    assert(serverProc != None)

    sock = create_socket()
    sock.send(b'blabla')
    time.sleep(3)
    res = recup_resp(sock)
    assert(res.status == 408)

def test_throughput():
    serverProc = launch_server('tests/json/timeout_through.json')
    assert(serverProc != None)

    sock = create_socket()
    sock.send(b'bla')
    time.sleep(3)
    res = recup_resp(sock)
    assert(res.status == 408)

def test_transaction_proxy():
    serverProc = launch_server('tests/json/timeout_proxy.json')
    assert(serverProc != None)
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("", 6666))
    sock.listen(5)

    sock = create_socket()
    make_request(sock, 'tests/custom/valid.txt')
    time.sleep(3)
    res = recup_resp(sock)
    assert(res.status == 504)
    