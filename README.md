# Messenger
TL;DR: A plugin to control [Task Spoolers](https://github.com/justanhduc/task-spooler) remotely in multiple servers.

## Introduction
On its own, Messenger does not do anything much interesting beyond 
parsing command lines and sending/receiving messages.
Messenger enables users to use Task Spooler remotely from their PCs.
Users can also control multiple Task Spoolers in multiple remote servers.
A scenario in which Messenger can be potentially useful is that 
users want to run some heavy task on remote servers. 
Messenger can automatically transfer the script and data via the `--sync` flag,
and then execute the task under Task Spooler.
This is what I do almost everyday, and is the motivation that urged me to 
develop Messenger in the first place.

## Installation
There are two separate steps to install Messenger properly.

- Install [Messenger-server](messenger-server/) in a remote server following the instructions.
- Install [Messenger-client](messenger-client/) as instructed.

## Known issues

Sometimes, a GPU task may randomly hang.

## Disclaimer

This does not have anything to do with Facebook's Messenger,
even though this Messenger does send and receive messages.

Most but not all the written modules are properly checked. 
No replacements or refunds for buggy performance. 
All PRs are welcome.

## Contact

Please contact me regarding anything related to Task Spooler and Messenger.
