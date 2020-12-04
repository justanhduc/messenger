# Messenger Client

Messenger client is the client part of the Messenger plugin for Task Spooler.
Messenger client opens a port in the local machine and send the task command,
and optionally data, to Messenger server, 
from which the message will be forwarded to Task Spooler.
Messenger client will then show any message received Task Spooler 
regarding the sent task.

## Installation

```
make install
```

## Usages

Make a `.servers_ports` in the home directory following the format below

```
<host1> <port1>
<host2> <port2>
...
```
`<host1>` and `<port1>` will be selected by default. 
Then try execute 

```
ms
``` 
The task queue of Task Spooler should appear.

The full list of commands is provided below.

```
>>> ms -h
usage: ms [messenger-flags] [task-spooler-flags] [command] 
       messenger-flags: [--cd directory] [--env FLAG=VALUE] [--host host_num] 
                        [--show_free_gpus] [--num_free_gpus] [--auto_server] 
                        [--kill] [--sync directory] [--excludes pattern1,pattern2,...] 
       task-spooler-flags: [-h] [--set_gpu_wait seconds] [--get_gpu_wait] [--get_label] 
                           [--count_running] [--last_queue_id] [--gpus num] [--full_cmd job_id] 
                           [-K] [-C] [-l] [-S num] [-t job_id] [-c job_id] [-p job_id] [-o job_id] 
                           [-i job_id] [-s job_id] [-r job_id] [-w job_id] [-k job_id] [-T] 
                           [-u job_id] [-U job_id1-job_id2] [-B] [-V] [-n] [-E] [-g] [-f] [-m] [-d] 
                           [-D job_id] [-L label] [-N num] 

Messenger - A multi-server plugin for Task Spooler

positional arguments:
  command               job to be run

optional arguments:
  -h, --help            show this help message and exit
  --cd directory        change directory. For e.g., ``ms --cd /home/justanhduc/Documents``.
  --env FLAG1=VALUE1:FLAG2=VALUE2:...
                        set environment variable flags.
  --host host_num, -H host_num
                        host to select. Value corresponds to the order specified in the ".servers_ports" file.
  --show_free_gpus      show current available GPUs info.
  --num_free_gpus       show the number of available GPUs.
  --auto_server         auto-magically choose a server based on the number of available GPUs.
  --kill                kill Messenger server.
  --sync directory      whether to sync the selected working directory to a temp directory before executing the
                        command.
  --excludes pattern1,pattern2,...
                        exception patterns when moving files to server.
  --set_gpu_wait seconds
                        set time to wait before running the next GPU job (30 seconds by default)
  --get_gpu_wait        get time to wait before running the next GPU job.
  --get_label, -a       show the job label. Of the last added, if not specified.
  --count_running, -R   return the number of running jobs
  --last_queue_id, -q   show the job ID of the last added.
  --full_cmd job_id     show full command. Of the last added, if not specified.
  -K                    kill the task spooler server
  -C                    clear the list of finished jobs
  -l                    show the job list (default action)
  -S num                get/set the number of max simultaneous jobs of the server.
  -t job_id             "tail -n 10 -f" the output of the job. Last run if -1.
  -c job_id             like -t, but shows all the lines. Last run if -1.
  -p job_id             show the pid of the job. Last run if -1.
  -o job_id             show the output file. Of last job run, if -1.
  -i job_id             show job information. Of last job run, if -1.
  -s job_id             show the job state. Of the last added, if -1.
  -r job_id             remove a job. The last added, if -1.
  -w job_id             wait for a job. The last added, if -1.
  -k job_id             send SIGTERM to the job process group. The last run, if -1.
  -T                    send SIGTERM to all running job groups.
  -u job_id             put that job first. The last added, if not specified.
  -U job_id1-job_id2    swap two jobs in the queue.
  -B                    in case of full queue on the server, quit (2) instead of waiting.
  -V                    show the program version
  -n                    don't store the output of the command.
  -E                    Keep stderr apart, in a name like the output file, but adding '.e'.
  -g                    gzip the stored output (if not -n).
  -f                    don't fork into background.
  -m                    send the output by e-mail (uses sendmail).
  -d                    the job will be run after the last job ends.
  -D job_id             the job will be run after the job of given id ends.
  -L label              name this task with a label, to be distinguished on listing.
  -N num                number of slots required by the job (1 default).
  --gpus num, -G num    number of GPUs required by the job (1 default).
```
