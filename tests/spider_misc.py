#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
import time
import socket
import http.client
import signal

def timeout_handler(signum, frame):
    raise Exception("end of time")

def setup_alarm(time):
    signal.signal(signal.SIGALRM, timeout_handler)
    signal.alarm(time)

def reset_alarm():
    signal.alarm(0)


def launch_server(json = "tests/json/test1.json"):
    os.system("pkill spider")

    setup_alarm(2)
    try :
        serverProc = subprocess.Popen(["./spider " + json],
                                  shell=True, preexec_fn=os.setsid,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        while serverProc.stderr.readline().decode("utf-8") != "Server launched !\n":
            continue
    except Exception:
        assert("Can't launch server" == "")
    reset_alarm()
    return serverProc

def kill_server(serverProc):
    assert(serverProc.poll() == None)
    os.killpg(os.getpgid(serverProc.pid), signal.SIGINT)

def custom_request(path, ip="127.0.0.1", port=8000):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ip, port))
    file_content = open(path, "rb").read()
    sock.send(file_content)
    time.sleep(0.2)
    setup_alarm(2)
    try:
        resp = http.client.HTTPResponse(sock)
        resp.begin()
    except Exception:
        assert("No response" == "")
    reset_alarm()
    return resp

def fragmented_request(path):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("127.0.0.1", 8000))
    file_content = open(path, "rb").read()
    i = 0
    while i < len(file_content) :
        sock.send(file_content[i:i+10])
        time.sleep(0.1)
        i += 10
    time.sleep(0.2)
    setup_alarm(2)
    try:
        resp = http.client.HTTPResponse(sock)
        resp.begin()
    except Exception:
        assert("No response" == "")
    reset_alarm()
    return resp
