from collections import namedtuple
import socket
import struct
import argparse
import csv
import os
import random
import string
import subprocess

Connection = namedtuple('Connection', ('hosts', 'ports'))
tmp_root = '/tmp/messenger-tmp'


class Argument:
    def __init__(self, argv):
        self.argv = list(argv)
        self.args = None
        self.parse_arguments()

    def parse_arguments(self):
        parser = argparse.ArgumentParser(prog='Messenger',
                                         description='Messenger - A multi-server plugin for Task Spooler')
        parser.add_argument('cmd', nargs='+', type=str, help='task spooler options')
        parser.add_argument('--cd', type=str, help='change directory. '
                                                   'For e.g., ``ms --cd /home/justanhduc/Documents``.')
        parser.add_argument('--env', type=str, help='set an environment variable flag. '
                                                    'For e.g., ``ms --cd CUDA_VISIBLE_DEVICES=0``.')
        parser.add_argument('--host', type=int, default=0, help='host to select. Value corresponds to the order '
                                                                'specified in the \".hosts_ports\" file.')
        parser.add_argument('--show_free_gpus', action='store_true', help='show current available GPUs info.')
        parser.add_argument('--num_free_gpus', action='store_true', help='show the number of available GPUs.')
        parser.add_argument('--auto_server', action='store_true', help='auto-magically choose a server based on '
                                                                       'the number of available GPUs.')
        parser.add_argument('--kill', action='store_true', help='kill Messenger server.')

        parser.add_argument('--set_gpu_wait', type=int, help='set time to wait before running the next GPU job '
                                                             '(30 seconds by default)')
        parser.add_argument('--get_gpu_wait', action='store_true',
                            help='get time to wait before running the next GPU job.')
        parser.add_argument('--get_label', '-a', action='store_true',
                            help='show the job label. Of the last added, if not specified.')
        parser.add_argument('--count_running', '-R', action='store_true', help='return the number of running jobs')
        parser.add_argument('--last_queue_id', '-q', action='store_true', help='show the job ID of the last added.')
        parser.add_argument('--gpus', '-G', type=int, help='number of GPUs required by the job (1 default).')
        parser.add_argument('--sync', type=str, help='whether to sync the selected working directory to'
                                                     'a temp directory before executing the command.')
        parser.add_argument('--excludes', type=str, default=[], help='exception patterns '
                                                                     'when moving files to server.')

        parser.add_argument('-K', action='store_true', help='kill the task spooler server')
        parser.add_argument('-C', action='store_true', help='clear the list of finished jobs')
        parser.add_argument('-l', action='store_true', help='show the job list (default action)')
        parser.add_argument('-S', type=int, help='get/set the number of max simultaneous jobs of the server.')
        parser.add_argument('-t', type=int, help='\"tail -n 10 -f\" the output of the job. Last run if -1.')
        parser.add_argument('-c', type=int, help='like -t, but shows all the lines. Last run if -1.')
        parser.add_argument('-p', type=int, help='show the pid of the job. Last run if -1.')
        parser.add_argument('-o', type=int, help='show the output file. Of last job run, if -1.')
        parser.add_argument('-i', type=int, help='show job information. Of last job run, if -1.')
        parser.add_argument('-s', type=int, help='show the job state. Of the last added, if -1.')
        parser.add_argument('-r', type=int, help='remove a job. The last added, if -1.')
        parser.add_argument('-w', type=int, help='wait for a job. The last added, if -1.')
        parser.add_argument('-k', type=int, help='send SIGTERM to the job process group. The last run, if -1.')
        parser.add_argument('-T', action='store_true', help='send SIGTERM to all running job groups.')
        parser.add_argument('-u', type=int, help='put that job first. The last added, if not specified.')
        parser.add_argument('-U', type=str, help='swap two jobs in the queue.')
        parser.add_argument('-B', action='store_true', help='in case of full queue on the server, '
                                                            'quit (2) instead of waiting.')
        parser.add_argument('-V', action='store_true', help='show the program version')

        parser.add_argument('-n', help='don\'t store the output of the command.')
        parser.add_argument('-E', help='Keep stderr apart, in a name like the output file, but adding \'.e\'.')
        parser.add_argument('-g', help='gzip the stored output (if not -n).')
        parser.add_argument('-f', help='don\'t fork into background.')
        parser.add_argument('-m', help='send the output by e-mail (uses sendmail).')
        parser.add_argument('-d', help='the job will be run after the last job ends.')
        parser.add_argument('-D', type=int, help='the job will be run after the job of given id ends.')
        parser.add_argument('-L', type=str, help='name this task with a label, to be distinguished on listing.')
        parser.add_argument('-N', type=int, help='number of slots required by the job (1 default).')
        self.args, _ = parser.parse_known_intermixed_args(self.argv)


class MessengerClient:
    def __init__(self, argv):
        home = os.environ['HOME']

        with open(os.path.join(home, '.servers_ports')) as file:
            reader = csv.reader(file, delimiter=' ')
            rows = [(host, int(port)) for host, port in reader]

        rows = list(zip(*rows))
        self.conn = Connection(rows[0], rows[1])
        self.arg = Argument(argv)
        self.socket = socket.socket()
        if self.arg.args.c is None and self.arg.args.t is None and \
                not self.arg.args.num_free_gpus and not self.arg.args.show_free_gpus:
            self.socket.settimeout(.1)

    def choose_server(self):
        cmd = ['ms', '--num_free_gpus']
        num_gpus = 0
        host_choice = 0
        for idx, addr in enumerate(zip(self.conn.hosts, self.conn.ports)):
            s = socket.socket()
            s.connect(addr)
            s.send(struct.pack('!i', len(cmd)))
            for i in range(len(cmd)):
                s.send(struct.pack('!i', len(cmd[i])))
                s.sendall(cmd[i].encode('utf8'))

            num_gpus_ = int(s.recv(1024).decode())
            num_gpus = max(num_gpus_, num_gpus)
            num_gpus = num_gpus if num_gpus > num_gpus_ else num_gpus_
            host_choice = host_choice if num_gpus > num_gpus_ else idx
            s.close()

        return host_choice

    def sync(self, host):
        tmpdir = os.path.join(tmp_root, 'tmp-')
        tmpdir += ''.join(random.choices(string.ascii_uppercase + string.digits, k=5))

        cmd = ['rsync', '-vuar', self.arg.args.sync, f'{host}:{tmpdir}']
        excludes = self.arg.args.excludes.split(',')
        exclude = [f'--exclude={exclude}' for exclude in excludes]
        cmd += exclude
        subprocess.call(cmd)  # sync using rsync
        return tmpdir

    def exec(self):
        if self.arg.args.auto_server:
            assert self.arg.args.gpus is not None, 'auto_server requires process to run with GPU.'

        choice = self.choose_server() if self.arg.args.auto_server else self.arg.args.host
        host = self.conn.hosts[choice]
        port = self.conn.ports[choice]
        if self.arg.args.sync is not None:
            assert self.arg.args.cd is None, '--sync overrides --cd.'
            tmpdir = self.sync(host)

            # change directory to the temp dir
            self.arg.argv.insert(1, tmpdir)
            self.arg.argv.insert(1, '--cd')

        self.socket.connect((host, port))
        self.socket.send(struct.pack('!i', len(self.arg.argv)))
        for i in range(len(self.arg.argv)):
            self.socket.send(struct.pack('!i', len(self.arg.argv[i])))
            self.socket.sendall(self.arg.argv[i].encode('utf8'))

        # receive data from the server
        while True:
            data = self.socket.recv(1024)
            if not data:
                break

            print(data.decode())

        self.socket.close()
