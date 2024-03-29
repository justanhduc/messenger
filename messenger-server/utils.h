/* Created by justanhduc on 11/6/20. */

#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/bind.hpp>
#include <boost/process.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/start_dir.hpp>
#include <getopt.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "logging.h"

#define BSIZE 1024

using boost::asio::ip::tcp;
namespace bp = boost::process;
using bp::posix::fd;

typedef boost::asio::detail::socket_option::boolean<SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT>
        reuse_port;
typedef std::vector<char *> cstrings;
typedef std::vector<int> ints;

extern Logger logger;

enum Actions {
    TASK_SPOOLER,
    KILL_SERVER
};

class Connection {
public:
    std::string host;
    int port{};

    Connection() = default;

    explicit Connection(const std::string &);
};

class Environment {
public:
    boost::process::native_environment env;

    Environment() : env(boost::this_process::environment()) {};

    void setGetoptEnv();

    void unsetGetoptEnv();

    void setEnv();

    void parseFlag(std::string flag);

private:
    strings envFlags;
    strings envFlagVals;
};

class Argument {
public:
    int redirectOutput;
    std::string currentDir;
    Connection conn;
    Environment env;
    Actions action;

    Argument() : redirectOutput(1), currentDir(std::getenv("HOME")), action(TASK_SPOOLER) {};

    explicit Argument(const std::string &filename)
            : redirectOutput(0), currentDir(std::getenv("HOME")) {
        conn = Connection(filename);
        env = Environment();
        action = TASK_SPOOLER;
    };

    int parseOpts(int argc, strings &argv);
};

static struct option longOptions[] = {
        {"cd",          required_argument, nullptr, 0},
        {"env",         required_argument, nullptr, 0},
        {"host",        required_argument, nullptr, 'H'},
        {"auto_server", no_argument,       nullptr, 0},
        {"kill",        no_argument,       nullptr, 0},
        {"sync",        required_argument, nullptr, 0},
        {"sync_dest",   required_argument, nullptr, 0},
        {"exclude",     required_argument, nullptr, 0},
        {"include",     required_argument, nullptr, 0},
        {"ln",          required_argument, nullptr, 0},
        {nullptr, 0,                       nullptr, 0}};

cstrings stringsToCstring(strings &);

strings splitString(std::string s, const std::string &delimiter);

void sendString(int fd, const std::string &string);

std::string recvString(int fd);

std::string buildTsCommand(strings &argv, int index);

bool isBooted(boost::asio::io_context &io_context, const std::string &filename);
