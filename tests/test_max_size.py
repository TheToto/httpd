#!/usr/bin/env python

# basic test architectures. Must be written in a test_*.py file

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

def test_uri_too_long_server():
    serverProc = launch_server('tests/json/max_size_server.json')
    assert(serverProc != None)
    requ = custom_request('tests/custom/max_size_uri.txt')
    assert(requ.status == 414)
    kill_server(serverProc)

def test_payload_too_large_server():
    serverProc = launch_server('tests/json/max_size_server.json')
    assert(serverProc != None)
    requ = custom_request('tests/custom/max_size_payload.txt')
    assert(requ.status == 413)
    kill_server(serverProc)

def test_header_fields_too_large_server():
    serverProc = launch_server('tests/json/max_size_server.json')
    assert(serverProc != None)
    requ = custom_request('tests/custom/max_size_header_fields.txt')
    assert(requ.status == 431)
    kill_server(serverProc)

def test_uri_too_long_proxy():
    proxyProc = launch_server('tests/json/max_size_proxy.json')
    assert(proxyProc != None)
    requ = custom_request('tests/custom/max_size_uri.txt')
    assert(requ.status == 414)
    kill_server(proxyProc)

def test_payload_too_large_proxy():
    proxyProc = launch_server('tests/json/max_size_proxy.json')
    assert(proxyProc != None)
    requ = custom_request('tests/custom/max_size_payload.txt')
    assert(requ.status == 413)
    kill_server(proxyProc)

def test_header_fields_too_large_proxy():
    proxyProc = launch_server('tests/json/max_size_proxy.json')
    assert(proxyProc != None)
    requ = custom_request('tests/custom/max_size_header_fields.txt')
    assert(requ.status == 431)
    kill_server(proxyProc)
