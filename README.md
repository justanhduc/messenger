# Messenger Server

## Introduction

Messenger server is the server part of the Messenger plugin for Task Spooler.
Messenger server opens a listening port on the remote server 
to wait for commands from Messenger client,
and passes the command "message" to Task Spooler. 
Any feedback from Task Spooler is redirected back to client via Messenger server.

## Requirements

[Boost]() (>=)

[CUDA]()

## Installation

Firstly, set a `CUDA_HOME` environment variable to point to the CUDA root.
Then execute

```
sudo ./install
```

## Usages 

In the home directory, make a `.hosts_ports` with the following format

```
xxx <port>
```
in which `<port>` is the listening port the server will open.

Next, boot the server by

```
ts_server
``` 
If the output says `Server has been booted`, then it's done!
