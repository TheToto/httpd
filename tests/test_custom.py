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

def test_http10_frag():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = fragmented_request("tests/custom/http10.txt")
    assert(resp.status == 426)
    kill_server(serverProc)


def test_grabage():
    serverProc = launch_server()
    assert(serverProc != None)
    resp = custom_request("tests/custom/grab.txt")
    assert(resp.status == 400)
    kill_server(serverProc)


def test_no_vhost():
    serverProc = launch_server("tests/json/test2.json")
    assert(serverProc != None)
    resp = custom_request("tests/custom/wrong_vhost.txt")
    assert(resp.status == 400)
    kill_server(serverProc)


def test_valid():
    serverProc = launch_server("tests/json/test2.json")
    assert(serverProc != None)
    resp = custom_request("tests/custom/valid.txt")
    assert(resp.status == 200)
    assert(resp.read().decode("utf-8") == open("tests/index.html", "r").read())
    kill_server(serverProc)

def test_valid_frag():
    serverProc = launch_server("tests/json/test2.json")
    assert(serverProc != None)
    resp = fragmented_request("tests/custom/valid.txt")
    assert(resp.status == 200)
    assert(resp.read().decode("utf-8") == open("tests/index.html", "r").read())
    kill_server(serverProc)


def test_valid_body():
    serverProc = launch_server("tests/json/test2.json")
    assert(serverProc != None)
    resp = custom_request("tests/custom/valid_withbody.txt")
    assert(resp.status == 200)
    assert(resp.read().decode("utf-8") == open("tests/index.html", "r").read())
    kill_server(serverProc)

def test_valid_body_frag():
    serverProc = launch_server("tests/json/test2.json")
    assert(serverProc != None)
    resp = fragmented_request("tests/custom/valid_withbody.txt")
    assert(resp.status == 200)
    assert(resp.read().decode("utf-8") == open("tests/index.html", "r").read())
    kill_server(serverProc)

def test_0x0_body():
    serverProc = launch_server("tests/json/test2.json")
    assert(serverProc != None)
    resp = custom_request("tests/custom/0x0_body.txt")
    assert(resp.status == 200)
    assert(resp.read() == open("tests/index.html", "rb").read())
    kill_server(serverProc)

def test_0x0_head():
    serverProc = launch_server("tests/json/test2.json")
    assert(serverProc != None)
    resp = custom_request("tests/custom/0x0_head.txt")
    assert(resp.status == 200)
    assert(resp.read() == open("tests/index.html", "rb").read())
    kill_server(serverProc)