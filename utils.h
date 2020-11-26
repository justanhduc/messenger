/* Created by justanhduc on 11/6/20. */

#pragma once

#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <vector>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/signal_set.hpp>
#include <sys/wait.h>
#include <boost/process/environment.hpp>
#include <boost/process.hpp>
#include <boost/process/start_dir.hpp>

#include "logging.h"

#define BSIZE 1024

using boost::asio::ip::tcp;
namespace bp = boost::process;
using bp::posix::fd;

typedef boost::asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT> reuse_port;
typedef std::vector<char *> cstrings;
typedef std::vector<int> ints;

extern Logging logging;

enum Actions {
    SHOW_FREE_GPUS,
    COUNT_FREE_GPUS,
    TASK_SPOOLER,
    KILL_SERVER
};

class Connection {
public:
    strings hosts;
    ints ports;
    int numHosts = 0;
    int hostNum = 0;

    Connection() : numHosts(0), hostNum(0) {};

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

    Argument() : redirectOutput(1), currentDir(std::getenv("HOME")) {};

    explicit Argument(const std::string &filename) :
            redirectOutput(0),
            currentDir(std::getenv("HOME")) {
        conn = Connection(filename);
        env = Environment();
    };

    int parseOpts(int argc, strings &argv);
};

static struct option longOptions[] = {
        {"cd",             required_argument, nullptr, 0},
        {"env",            required_argument, nullptr, 0},
        {"host",           required_argument, nullptr, 0},
        {"show_free_gpus", no_argument,       nullptr, 0},
        {"num_free_gpus",  no_argument,       nullptr, 0},
        {"auto_server",    no_argument,       nullptr, 0},
        {"kill",           no_argument,       nullptr, 0},
        {"sync",           required_argument, nullptr, 0},
        {"exclude",        required_argument, nullptr, 0},
        {nullptr, 0,                          nullptr, 0}
};

static inline void report(const std::string &msg) {
    std::fprintf(stderr, "%s\n", msg.c_str());
}

cstrings stringsToCstring(strings &);

strings splitString(std::string s, const std::string &delimiter);

void sendString(int fd, const std::string &string);

std::string recvString(int fd);

std::string buildTsCommand(strings &argv, int index);

bool isBooted(boost::asio::io_context &io_context, const std::string &filename);
