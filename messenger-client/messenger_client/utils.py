from collections import namedtuple
import socket
import struct
import argparse
import csv
import os
import random
import string
import subprocess

Connection = namedtuple('Connection', ('host', 'port'))
tmp_root = '/tmp/messenger-tmp'


def get_usage():
    usage = "ms [messenger-flags] [task-spooler-command] \n"
    usage += "       messenger-flags: [--cd directory] [--env FLAG1=VALUE1:FLAG2=VALUE2:...] [--host/-H host_name] \n" \
             "                        [--kill] [--sync directory] [--sync_dest directory] [--exclude pattern1:pattern2:...] \n"

    return usage


class Argument:
    def __init__(self, argv):
        self.argv = list(argv)
        self.args = None
        self.cmd = None
        self.parse_arguments()

    def parse_arguments(self):
        parser = argparse.ArgumentParser(prog='Messenger',
                                         description='Messenger - A multi-server plugin for Task Spooler',
                                         usage=get_usage())
        parser.add_argument('ts_cmd', metavar='command', type=str, help='task spooler command')
        parser.add_argument('--cd', metavar='directory', type=str,
                            help='change directory. For e.g., ``ms --cd /home/justanhduc/Documents``.')
        parser.add_argument('--env', metavar='\"FLAG1=VALUE1;FLAG2=VALUE2;...\"', type=str,
                            help='set environment variable flags.')
        parser.add_argument('--host', '-H', metavar='host_name', type=str, default=None,
                            help='host to select. Value corresponds to the order '
                                 'specified in the \".servers_ports\" file.')
        parser.add_argument('--kill', action='store_true', help='kill Messenger server.')
        parser.add_argument('--sync', metavar='directory', type=str,
                            help='whether to sync the selected working directory to '
                                 'a temp directory before executing the command.')
        parser.add_argument('--sync_dest', metavar='directory', type=str,
                            help='sync destination on the remote server.')
        parser.add_argument('--exclude', metavar='pattern1:pattern2:...', type=str, default='',
                            help='exception patterns when moving files to server.')
        self.args, self.cmd = parser.parse_known_intermixed_args(self.argv)
        print('ts '+ ' '.join(self.cmd))


class MessengerClient:
    def __init__(self, argv):
        home = os.environ['HOME']

        with open(os.path.join(home, '.servers_ports')) as file:
            reader = csv.reader(file, delimiter=' ')
            rows = [(name, Connection(host, int(port))) for name, host, port in reader]

        self.default_host = rows[0][0]
        self.conn = dict(rows)
        self.arg = Argument(argv)
        self.socket = socket.socket()

    def sync(self, host, target=None):
        if target is None:
            tmpdir = os.path.join(tmp_root, 'tmp-')
            tmpdir += ''.join(random.choices(string.ascii_uppercase + string.digits, k=5))
        else:
            tmpdir = target

        if self.arg.args.exclude:
            excludes = self.arg.args.exclude.split(':')
            exclude = [f'--exclude={exclude}' for exclude in excludes]
        else:
            exclude = []

        syncs = self.arg.args.sync.split(':')
        for to_sync in syncs:
            cmd = ['rsync', '-uar', to_sync]
            cmd += exclude
            cmd += [f'{host}:{tmpdir}/']
            subprocess.call(cmd)  # sync using rsync

        return tmpdir

    def exec(self):
        choice = self.arg.args.host if self.arg.args.host is not None else self.default_host
        host = self.conn[choice].host
        port = self.conn[choice].port
        if self.arg.args.sync is not None:
            tmpdir = self.sync(host, self.arg.args.sync_dest)

            # change directory to the temp dir
            if self.arg.args.cd is None:
                self.arg.argv.insert(1, tmpdir)
                self.arg.argv.insert(1, '--cd')

        self.socket.connect((host, port))
        print(f'Connected to {host}:{port}')
        self.socket.send(struct.pack('!i', len(self.arg.argv)))
        for i in range(len(self.arg.argv)):
            self.socket.send(struct.pack('!i', len(self.arg.argv[i])))
            self.socket.sendall(self.arg.argv[i].encode('utf8'))

        # receive data from the server
        while True:
            data = self.socket.recv(1024)
            if not data:
                break

            print(data.decode(), end='')

        self.socket.close()
