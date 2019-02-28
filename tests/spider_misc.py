#!/usr/bin/env python

import pytest
import requests
import subprocess
import os
import signal
import time

def launch_server():
    os.system("killall spider")
    timeout = time.time() + 2 # 2 seconds
    serverProc = subprocess.Popen(["./spider tests/json/test1.json"],
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