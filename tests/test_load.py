#!/usr/bin/env python

# basic test architectures. Must be written in a test_*.py file

import pytest
import requests
import subprocess
import os
import signal
import time
from spider_misc import *

def test_round_robin_simple():
    serverProc = launch_server('tests/json/round-robin/round-robin.json')
    serverBack1 = launch_server('tests/json/round-robin/round-robin-backend1.json', False)
    serverBack2 = launch_server('tests/json/round-robin/round-robin-backend2.json', False)
    assert(serverProc != None)
    assert(serverBack1 != None)
    assert(serverBack2 != None)

    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)

    kill_server(serverBack1)
    kill_server(serverBack2)
    kill_server(serverProc)



def test_round_robin_no_back():
    serverProc = launch_server('tests/json/round-robin/round-robin.json')
    assert(serverProc != None)

    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 502)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 502)

    kill_server(serverProc)


def test_round_robin_one_back_1():
    serverProc = launch_server('tests/json/round-robin/round-robin.json')
    serverBack1 = launch_server('tests/json/round-robin/round-robin-backend1.json', False)
    assert(serverProc != None)
    assert(serverBack1 != None)

    requ = custom_request('tests/custom/valid.txt')
    requ2 = custom_request('tests/custom/valid.txt')
    assert((requ.status == 200 and requ2.status == 502) or (requ2.status == 200 and requ.status == 502))

    kill_server(serverBack1)
    kill_server(serverProc)

def test_round_robin_one_back_2():
    serverProc = launch_server('tests/json/round-robin/round-robin.json')
    serverBack1 = launch_server('tests/json/round-robin/round-robin-backend2.json', False)
    assert(serverProc != None)
    assert(serverBack1 != None)

    requ = custom_request('tests/custom/valid.txt')
    requ2 = custom_request('tests/custom/valid.txt')
    assert((requ.status == 200 and requ2.status == 502) or (requ2.status == 200 and requ.status == 502))

    kill_server(serverBack1)
    kill_server(serverProc)

def test_round_robin_loosing_1():
    serverProc = launch_server('tests/json/round-robin/round-robin.json')
    serverBack1 = launch_server('tests/json/round-robin/round-robin-backend1.json', False)
    serverBack2 = launch_server('tests/json/round-robin/round-robin-backend2.json', False)
    assert(serverProc != None)
    assert(serverBack1 != None)
    assert(serverBack2 != None)

    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)

    kill_server(serverBack1)

    requ = custom_request('tests/custom/valid.txt')
    requ2 = custom_request('tests/custom/valid.txt')
    assert((requ.status == 200 and requ2.status == 502) or (requ2.status == 200 and requ.status == 502))
    kill_server(serverBack2)
    kill_server(serverProc)


def test_round_robin_loosing_2():
    serverProc = launch_server('tests/json/round-robin/round-robin.json')
    serverBack1 = launch_server('tests/json/round-robin/round-robin-backend1.json', False)
    serverBack2 = launch_server('tests/json/round-robin/round-robin-backend2.json', False)
    assert(serverProc != None)
    assert(serverBack1 != None)
    assert(serverBack2 != None)

    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)

    kill_server(serverBack2)

    requ = custom_request('tests/custom/valid.txt')
    requ2 = custom_request('tests/custom/valid.txt')
    assert((requ.status == 200 and requ2.status == 502) or (requ2.status == 200 and requ.status == 502))

    kill_server(serverBack1)
    kill_server(serverProc)

def test_round_robin_loosing_both():
    serverProc = launch_server('tests/json/round-robin/round-robin.json')
    serverBack1 = launch_server('tests/json/round-robin/round-robin-backend1.json', False)
    serverBack2 = launch_server('tests/json/round-robin/round-robin-backend2.json', False)
    assert(serverProc != None)
    assert(serverBack1 != None)
    assert(serverBack2 != None)

    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 200)

    kill_server(serverBack2)

    requ = custom_request('tests/custom/valid.txt')
    requ2 = custom_request('tests/custom/valid.txt')
    assert((requ.status == 200 and requ2.status == 502) or (requ2.status == 200 and requ.status == 502))

    kill_server(serverBack1)

    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 502)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 502)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 502)
    requ = custom_request('tests/custom/valid.txt')
    assert(requ.status == 502)
    kill_server(serverProc)

