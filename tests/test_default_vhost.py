#!/usr/bin/env python

# basic test architectures. Must be written in a test_*.py file

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

def test_no_default_vhost_server():
    serverProc = launch_server('tests/json/proxy.json')
    assert(serverProc != None)
    requ = custom_request('tests/custom/default_vhost.txt', 'sub.localhost', 8080)
    assert(requ.status == 400)
    kill_server(serverProc)

def test_default_vhost_server():
    serverProc = launch_server('tests/json/server_default_vhost.json')
    assert(serverProc != None)
    requ = custom_request('tests/custom/default_vhost.txt', 'sub.localhost', 8080)
    assert(requ.status == 200)
    kill_server(serverProc)
