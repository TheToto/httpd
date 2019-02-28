#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

def test_http10():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = custom_request("tests/custom/http10.txt")
    assert(resp.status == 426)
    kill_server(serverProc)

def test_http09():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = custom_request("tests/custom/http09.txt")
    assert(resp.status == 426)
    kill_server(serverProc)

def test_http11():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = custom_request("tests/custom/http11.txt")
    assert(resp.status == 200)
    kill_server(serverProc)

def test_http20():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = custom_request("tests/custom/http20.txt")
    assert(resp.status == 505)
    kill_server(serverProc)
