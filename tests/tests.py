#!/usr/bin/env python

import os

import subprocess
import signal
from optparse import OptionParser

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

'''
parser = OptionParser()
parser.add_option("-l", "--list", action="store_true", dest="list", default=False)
parser.add_option("-c", "--category", dest="category")
parser.add_option("-s", "--sanity", action="store_true", dest="sanity", default=False)

(options, args) = parser.parse_args()


if options.list:
    print(bcolors.UNDERLINE + "\nlist of test categories:" + bcolors.ENDC)
    for dirname in dirs:
        print(bcolors.HEADER + "\n" + dirname + bcolors.ENDC)
        for filename in os.listdir('tests/commands/' + dirname):
            print("    -" + filename)
    print("\n\nCall them with ./tests/all_tests.py -c <subdirectory>/<file>\n")
    exit(0)


if options.category:
    options.category = "tests/commands/" + options.category
    try:
        with open(options.category, "r") as commands:
            for command in commands:
                tcommands.tcommands(options.category, options.sanity)
    except FileNotFoundError:
        print(bcolors.FAIL + "\nError: " + bcolors.WARNING + "test file '" + \
              bcolors.ENDC + options.category + bcolors.WARNING + \
              "' not found\n" + bcolors.ENDC)
    exit(0)
'''


procSpider = subprocess.Popen(["./spider", "tests/example.json"])
i = 0

while i < 30:
    i += 1
    print("Number of accepted socket: " + bcolors.OKGREEN + str(i) + bcolors.ENDC)
    pro = subprocess.Popen(["curl", "localhost:8000/tests/test.html"],
                           stderr=subprocess.PIPE,
                           stdin=subprocess.PIPE)
    pro.kill()


proip6 = subprocess.Popen(["curl", "-6", "localhost:8000/tests/test.html"],
                          stderr=subprocess.PIPE,
                          stdin=subprocess.PIPE)


