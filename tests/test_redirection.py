#!/usr/bin/env python

# basic test architectures. Must be written in a test_*.py file

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

def test_moved_permanently():
    serverProc = launch_server("tests/json/redirect.json")
    assert(serverProc != None)
    resp = requests.head('http://127.0.0.1:8080/secret')
    assert(resp.status_code == 301)
    kill_server(serverProc)
