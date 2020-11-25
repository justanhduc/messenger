# Messenger Client

Messenger client is the client part of the Messenger plugin for Task Spooler.
Messenger client opens a port in the local machine and send the task command,
and optionally data, to Messenger server, 
from which the message will be forwarded to Task Spooler.
Messenger client will then show any message received Task Spooler 
regarding the sent task.

## Installation

```
python setup.py install
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
--cd                  change directory. For e.g., ``ms --cd /home/justanhduc/Documents``
--env                 set an environment variable flag.For e.g., ``ms --cd CUDA_VISIBLE_DEVICES=0``
--host                Value corresponds to the order specified in the ".hosts_ports" file (default 0)
--show_free_gpus      show current available GPUs info
--num_free_gpus       show the number of available GPUs
--auto_server         auto-magically choose a server based on the number of available GPUs (must go with -G)
--kill                kill Messenger server

--sync                sync the selected working directory to a temp directory before executing the command
--excludes            exception pattern when moving files to server

                  -K  kill the task spooler serve
                  -C  clear the list of finished job
                  -l  show the job list (default action
                  -S  get/set the number of max simultaneous jobs of the server
                  -t  tail -n 10 -f" the output of the job. Last run if -1
                  -c  like -t, but shows all the lines. Last run if -1
                  -p  show the pid of the job. Last run if -1
                  -o  show the output file. Of last job run, if -1
                  -i  show job information. Of last job run, if -1
                  -s  show the job state. Of the last added, if -1
                  -r  remove a job. The last added, if -1
                  -w  wait for a job. The last added, if -1
                  -k  send SIGTERM to the job process group. The last run, if -1
                  -T  send SIGTERM to all running job groups
                  -u  put that job first. The last added, if not specified
                  -U  swap two jobs in the queue
                  -B  in case of full queue on the server quit (2) instead of waiting
--set_gpu_wait        set time to wait before running the next GPU job (30 seconds by default)
--get_gpu_wait        time to wait before running the next GPU job
--get_label       -a  show the job label. Of the last added, if not specified
--count_running   -R  return the number of running job
--last_queue_id   -q  show the job ID of the last added
                  -V  show the program version

                  -n  don't store the output of the command
                  -E  Keep stderr apart, in a name like the output file, but adding '.e'
                  -g  gzip the stored output (if not -n)
                  -f  don't fork into background
                  -m  send the output by e-mail (uses sendmail)
                  -d  the job will be run after the last job ends
                  -D  the job will be run after the job of given id ends
                  -L  name this task with a label, to be distinguished on listing
                  -N  number of slots required by the job (1 default)
--gpus            -G  number of GPUs required by the job (1 default)
```
