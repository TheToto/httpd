#!/usr/bin/env python

# basic test architectures. Must be written in a test_*.py file

import pytest
import requests
import subprocess
import os
import signal

# must be present in a function "test_*" ; You can write multiple functions
def test_simple():

    # this is the server process
    #              change json location here ->
    serverProc = subprocess.Popen(["./spider tests/json/test1.json"],
                                  shell=True, preexec_fn=os.setsid)

    # for example, a simple get request to tests/test.page
    # you can do way more varied tests with the Requests library

    requ = requests.get('http://127.0.0.1:8000/tests/test.page')
    expectedStr = open("tests/test.page", "r").read()

    # write tests as assertions here
    assert(requ.text == expectedStr)
    assert(requ.status_code == 200)
    assert(requ.headers['content-length'] == str(len(expectedStr)))

    # use this command to kill the server
    os.killpg(os.getpgid(serverProc.pid), signal.SIGTERM)

def test_404():
    serverProc = subprocess.Popen(["./spider tests/json/test1.json"],
                                  shell=True, preexec_fn=os.setsid)
    requ1 = requests.get('http://127.0.0.1:8000/tests/doesnotexist')
    requ2 = requests.head('http://127.0.0.1:8000/tests/doesnotexist')
    requ3 = requests.post('http://127.0.0.1:8000/tests/doesnotexist')
    requ4 = requests.put('http://127.0.0.1:8000/tests/doesnotexist')

    assert(requ1.status_code == 404)
    assert(requ2.status_code == 404)
    assert(requ3.status_code == 404)

    assert(requ2.headers['content-length'] == "0")

    assert(requ4.status_code == 405) # Method not Allowed

    os.killpg(os.getpgid(serverProc.pid), signal.SIGTERM)
