#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
from spider_misc import *

def test_get_CL0():
    serverProc = launch_server()
    assert(serverProc != None)
    response = custom_request("tests/custom/length_get_CL0.txt")
    assert(response.status == 200)
    kill_server(serverProc)

def test_get_CL10():
    serverProc = launch_server()
    assert(serverProc != None)
    response = custom_request("tests/custom/length_get_CL10.txt")
    assert(response.status == 400)
    kill_server(serverProc)

def test_head_CL0():
    serverProc = launch_server()
    assert(serverProc != None)
    response = custom_request("tests/custom/length_head_CL0.txt")
    assert(response.status == 200)
    kill_server(serverProc)

def test_head_CL10():
    serverProc = launch_server()
    assert(serverProc != None)
    response = custom_request("tests/custom/length_head_CL10.txt")
    assert(response.status == 400)
    kill_server(serverProc)

def test_post_CL0():
    serverProc = launch_server()
    assert(serverProc != None)
    response = custom_request("tests/custom/length_post_CL0.txt")
    assert(response.status == 200)
    kill_server(serverProc)

def test_post_CL10():
    serverProc = launch_server()
    assert(serverProc != None)
    response = custom_request("tests/custom/length_post_CL10.txt");
    assert(response.status == 200)
    kill_server(serverProc)

def test_post_CL_invalid():
    serverProc = launch_server()
    assert(serverProc != None)
    response = custom_request("tests/custom/length_post_CL_invalid.txt");
    assert(response.status == 400)
    kill_server(serverProc)
