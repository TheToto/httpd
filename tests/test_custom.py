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