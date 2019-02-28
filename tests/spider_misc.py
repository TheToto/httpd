#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
import time
import socket
import http.client

def launch_server(json = "tests/json/test1.json"):
    os.system("killall spider")
    timeout = time.time() + 2 # 2 seconds
    serverProc = subprocess.Popen(["./spider " + json],
                                  shell=True, preexec_fn=os.setsid,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    while serverProc.stderr.readline().decode("utf-8") != "Server launched !\n":
        time.sleep(0.3)
        if time.time() > timeout:
            return None
        continue
    
    return serverProc

def kill_server(serverProc):
    os.killpg(os.getpgid(serverProc.pid), signal.SIGINT)

def custom_request(path):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("127.0.0.1", 8000))
    file_content = open(path, "rb").read()
    sock.send(file_content)
    time.sleep(0.2)
    resp = http.client.HTTPResponse(sock)
    resp.begin()
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
    resp = http.client.HTTPResponse(sock)
    resp.begin()
    return resp