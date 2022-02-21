#!/usr/bin/env python3
"""Download and setup problems from Competitive Companion

Usage:
    downloadprob.py [TEMPLATE] [-n NUMBER] [-b BATCHES] [-t TIMEOUT] [--echo] [--dryrun]

Options:
    -h --help     Show this screen.
    --echo        Echo received responses
    --dryrun      Echo the actions that would be performed.

Download limit options:
    -n NUMBER, --number NUMBER     Number of problems.
    -b BATCHES, --batches BATCHES  Number of batches. (Default 1 batch)
    -t TIMEOUT, --timeout TIMEOUT  Timeout for listening to problems, in seconds
"""
# Adapted from ecnerwala's of course

from docopt import docopt
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import subprocess
import re

ADDRESS = ('127.0.0.1', 10043)
TIMEOUT = 60
TEMPLATE = 'cpp'
ECHO = False
DRYRUN = False
NUMBER = 0
BATCHES = 0


class DownloadErrors:
    def fail(s):
        print(f'downloadprob: {s}', file=sys.stderr, flush=True)
        sys.exit(2)

    def io_error(file, err):
        DownloadErrors.fail(f'{file}: {err}')

    def empty_name(name):
        DownloadErrors.fail(f"empty folder name: '{name}'")

    def too_many_batches(expected, ids):
        DownloadErrors.fail(f'expected only {expected} batches: {ids}')

    def too_many_problems(expected, id):
        DownloadErrors.fail(f'expected only {expected} problems in batch {id}')


def listen_once():
    data = None

    class CompetitiveCompanionHandler(BaseHTTPRequestHandler):
        def do_POST(self):
            nonlocal data
            data = json.load(self.rfile)

    with HTTPServer(ADDRESS, CompetitiveCompanionHandler) as server:
        if ECHO:
            print('Listening...')
        server.timeout = TIMEOUT
        server.handle_request()

    if ECHO:
        print(json.dumps(data)) if data is not None else print('No data...')
    return data


def listen_many(callback):
    # Get by # batches
    if BATCHES > 0 and NUMBER == 0:
        progress, batch_ids = {}, []
        pending, problems = BATCHES, 0

        while pending > 0:
            print(f'Waiting for {pending} batches, {problems} problems...')
            data = listen_once()

            batch = data['batch']
            id, size = batch['id'], batch['size']
            if id not in batch_ids:
                if len(batch_ids) == pending:
                    DownloadErrors.too_many_batches(pending, batch_ids)
                batch_ids.append(id)
                progress[id] = [size, size]
                problems += size
            if progress[id][0] == 0:
                DownloadErrors.too_many_problems(progress[id][1], id)

            progress[id][0] -= 1
            problems -= 1
            pending -= progress[id][0] == 0

            callback(data)

    # Get by # problems
    elif NUMBER > 0:
        for i in range(NUMBER):
            if (data := listen_once()) is not None:
                callback(data)

    # Get until timeout
    else:
        while (data := listen_once()) is not None:
            callback(data)


def makeprob(name):
    if DRYRUN:
        print(f'makeprob "{name}" "{TEMPLATE}"')
        return True
    else:
        return subprocess.call(f'makeprob "{name}" "{TEMPLATE}"',
                               shell=True) == 0


def fillprob(data):
    name = data['name']
    folder = re.sub('[^a-zA-Z0-9-\+]+', '', name.lower().replace(' ', '-'))
    if len(folder) == 0:
        DownloadErrors.empty_name(name)
    if not makeprob(folder):
        return

    print(f"========= {name}")
    tests = data.get('tests', [])
    T = 0
    for test in tests:
        if 'input' not in test or 'output' not in test:
            continue
        T += 1
        infile, outfile = f'{folder}/{T}.in', f'{folder}/{T}.out'
        print(f'{infile}:\n{test["input"]}')
        print(f'{outfile}:\n{test["output"]}')
        if DRYRUN:
            continue
        try:
            with open(infile, 'w') as file:
                file.write(test['input'])
        except IOError as err:
            DownloadErrors.io_error(infile, err)
        try:
            with open(outfile, 'w') as file:
                file.write(test['output'])
        except IOError as err:
            DownloadErrors.io_error(outfile, err)


def notnone(x, default=0):
    return x if x is not None else default


def downloadprob():
    args = docopt(__doc__)

    global TIMEOUT, TEMPLATE, ECHO, DRYRUN, NUMBER, BATCHES
    TIMEOUT = int(notnone(args.get('--timeout'), TIMEOUT))
    TEMPLATE = str(notnone(args.get('TEMPLATE'), TEMPLATE))
    ECHO = bool(notnone(args.get('--echo'), False))
    DRYRUN = bool(notnone(args.get('--dryrun'), False))
    NUMBER = int(notnone(args.get('--number'), 0))
    BATCHES = int(notnone(args.get('--batches'), 1))

    listen_many(fillprob)


if __name__ == "__main__":
    downloadprob()
