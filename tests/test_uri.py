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


def test_uri_spaces():
    get_uri('tests/garbage/ a  ', 'tests/garbage/%20a%20%20')
    get_uri('tests/garbage/   ', 'tests/garbage/%20%20%20')

def test_uri_quotes():
    get_uri('tests/garbage/""', 'tests/garbage/%22%22')

def test_uri_question():
    get_uri('tests/garbage/?toto', 'tests/garbage/%3ftoto')

def test_uri_hardcore():
    get_uri('tests/garbage/#$%&@@@', 'tests/garbage/%23%24%25%26%40%40%40')


def test_uri_percent00_escape():
    get_uri('tests/garbage/%00', 'tests/garbage/%25%30%30')

def get_uri(path, pathURI):
    f = open(path, "w+")
    f.write("'" + path + "' content")
    f.close()

    serverProc = launch_server()
    assert(serverProc != None)

    requ = requests.get("http://127.0.0.1:8000/" + pathURI)
    expectedStr = open(path, "r").read()

    assert(requ.text == expectedStr)
    assert(requ.status_code == 200)
    assert(requ.headers['content-length'] == str(len(expectedStr)))

    kill_server(serverProc)
    os.remove(path)




def test_head_simple():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = requests.head('http://127.0.0.1:8000')
    assert(resp.status_code == 200)
    kill_server(serverProc)

def test_post_simple():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = requests.post('http://127.0.0.1:8000')
    assert(resp.status_code == 200)
    kill_server(serverProc)
