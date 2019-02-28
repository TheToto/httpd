#!/usr/bin/env python

# basic test architectures. Must be written in a test_*.py file

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

# must be present in a function "test_*" ; You can write multiple functions
def test_simple():

    # this is the server process
    #              change json location here ->
    serverProc = launch_server()
    assert(serverProc != None)
    # for example, a simple get request to tests/test.page
    # you can do way more varied tests with the Requests library

    requ = requests.get('http://127.0.0.1:8000/tests/test.page')
    expectedStr = open("tests/test.page", "r").read()

    # write tests as assertions here
    assert(requ.text == expectedStr)
    assert(requ.status_code == 200)
    assert(requ.headers['content-length'] == str(len(expectedStr)))

    # use this command to kill the server
    kill_server(serverProc)