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
        parser.add_argument('--include', metavar='pattern1:pattern2:...', type=str, default='',
                            help='patterns to include when moving files to server.')
        parser.add_argument('--exclude', metavar='pattern1:pattern2:...', type=str, default='',
                            help='patterns to exclude when moving files to server.')
        parser.add_argument('--ln', metavar='directory', type=str,
                            help='directory to be linked. Must be absolute.')
        parser.add_argument('--ln_dest', metavar='directory', type=str,
                            help='destination of the link.')
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

    def sync(self, host):
        target = self.arg.args.sync_dest
        if target is None:
            tmpdir = os.path.join(tmp_root, 'msg-')
            tmpdir += ''.join(random.choices(string.ascii_uppercase + string.digits, k=5))
        else:
            tmpdir = target

        if self.arg.args.exclude:
            excludes = self.arg.args.exclude.split(':')
            exclude = [f'--exclude={exc}' for exc in excludes]
        else:
            exclude = []

        if self.arg.args.include:
            includes = self.arg.args.include.split(':')
            include = [f'--include={inc}' for inc in includes]
            if not exclude:
                exclude = ['--exclude=*']
        else:
            include = []

        syncs = self.arg.args.sync.split(':')
        for to_sync in syncs:
            cmd = ['rsync', '-uar', to_sync]
            cmd += include
            cmd += exclude
            if host in ('localhost', '127.0.0.1'):
                cmd += [f'{tmpdir}/']
            else:
                cmd += [f'{os.getlogin()}@{host}:{tmpdir}/']

            subprocess.call(cmd)  # sync using rsync

        return tmpdir

    def create_symlink(self, host, cur_dir=None):
        target = self.arg.args.ln
        link = self.arg.args.ln_dest
        assert os.path.isabs(target), 'Linked directory must be absolute'
        if not os.path.isabs(link):
            assert cur_dir is not None or self.arg.args.cd is not None, \
                'Link is relative, but no destination is specified.' \
                'Where is it relative to?'
            dest = cur_dir if cur_dir is not None else self.arg.args.cd
            link = os.path.join(dest, link)

        if host in ('localhost', '127.0.0.1'):
            command = f"ln -s {target} {link}"
        else:
            command = f"ssh {os.getlogin()}@{host} 'ln -s {target} {link}'"

        subprocess.call(command, shell=True)

    def exec(self):
        choice = self.arg.args.host if self.arg.args.host is not None else self.default_host
        host = self.conn[choice].host
        port = self.conn[choice].port
        if self.arg.args.sync is not None:
            tmpdir = self.sync(host)

            # change directory to the temp dir
            if self.arg.args.cd is None:
                self.arg.argv.insert(1, tmpdir)
                self.arg.argv.insert(1, '--cd')
        else:
            tmpdir = None

        if self.arg.args.ln is not None and self.arg.args.ln_dest is not None:
            self.create_symlink(host, tmpdir)

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
