/* Created by justanhduc on 11/6/20. */

#include <sys/socket.h>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>

#include "utils.h"

Connection::Connection(const std::string &filename) {
    std::ifstream file(filename);
    std::string host;
    int port;
    numHosts = 0;
    if (!file.is_open())
        report("Cannot open host file");
    else {
        while (file >> host >> port) {
            hosts.push_back(host);
            ports.push_back(port);
            numHosts++;
        }
    }
    hostNum = 0;
}

void Environment::setEnv() {
    auto itFlag = envFlags.begin();
    auto itVal = envFlagVals.begin();
    for (itFlag = envFlags.begin(), itVal = envFlagVals.begin();
         (itFlag != envFlags.end()) && (itVal != envFlagVals.end()); ++itFlag, ++itVal)
        env[*itFlag] = *itVal;
}

void Environment::parseFlag(std::string flag) {
    strings flagsVals = splitString(flag, "=");
    envFlags.push_back(flagsVals[0]);
    envFlagVals.push_back(flagsVals[1]);
}

strings splitString(std::string s, const std::string &delimiter) {
    strings ss;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        ss.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    ss.push_back(s);
    return ss;
}

int Argument::parseOpts(int argc, strings &argv) {
    int c;
    int cmdIdx = 1;
    int optionIdx = 0;
    cstrings cargv = stringsToCstring(argv);

    action = TASK_SPOOLER;
    /* Parse options */
    while (true) {
        c = getopt_long(argc, cargv.data(), ":RVhKgClnfmBEr:a:t:c:o:p:w:k:u:s:U:qi:N:L:dS:D:",
                        longOptions, &optionIdx);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                if (strcmp(longOptions[optionIdx].name, "cd") == 0) {
                    currentDir = optarg;
                    cmdIdx += 2;
                } else if (strcmp(longOptions[optionIdx].name, "host") == 0) {
                    conn.hostNum = atoi(optarg);
                    if (conn.hostNum > conn.numHosts - 1) {
                        logging.log(
                                "There are only %d hosts but chose host %d (zero-based)", conn.numHosts, conn.hostNum);
                        exit(-1);
                    }
                    cmdIdx += 2;
                } else if (strcmp(longOptions[optionIdx].name, "env") == 0) {
                    env.parseFlag(optarg);
                    cmdIdx += 2;
                } else if (strcmp(longOptions[optionIdx].name, "auto_server") == 0) {
                    cmdIdx += 1;
                } else if (strcmp(longOptions[optionIdx].name, "exclude") == 0 ||
                           strcmp(longOptions[optionIdx].name, "sync") == 0) {
                    cmdIdx += 2;
                } else if (strcmp(longOptions[optionIdx].name, "num_free_gpus") == 0) {
                    action = COUNT_FREE_GPUS;
                } else if (strcmp(longOptions[optionIdx].name, "show_free_gpus") == 0) {
                    action = SHOW_FREE_GPUS;
                } else if (strcmp(longOptions[optionIdx].name, "kill") == 0) {
                    action = KILL_SERVER;
                } else
                    logging.log("Unknown option %s", longOptions[optionIdx].name);
                break;
        }
    }
    return cmdIdx;
}

cstrings stringsToCstring(strings &str) {
    cstrings cstrings;
    cstrings.reserve(str.size());

    for (auto &s: str)
        cstrings.push_back(&s[0]);
    return cstrings;
}

void sendString(int fd, const std::string &string) {
    int res;
    uint32_t size = htonl(string.length());
    res = send(fd, &size, sizeof(uint32_t), MSG_CONFIRM);
    if (res < 0)
        report("Cannot send package");

    res = send(fd, string.c_str(), string.size(), MSG_CONFIRM);
    if (res < 0)
        report("Cannot send package");
}

std::string recvString(int fd) {
    int res;
    uint32_t size;
    res = recv(fd, &size, sizeof(uint32_t), 0);
    if (res == -1) {
        report("Cannot receive package");
    }
    size = ntohl(size);

    std::vector<char> rcvBuf;    // Allocate a receive buffer
    rcvBuf.resize(size, 0x00); // with the necessary size
    res = recv(fd, &(rcvBuf[0]), size, 0); // Receive the string data
    if (res == -1)
        report("Cannot receive package");

    std::string string;
    string.assign(&(rcvBuf[0]), rcvBuf.size());
    return string;
}

std::string buildTsCommand(strings &argv, int index) {
    std::string cmd = "ts ";
    for (auto it = argv.begin() + index; it != argv.end(); ++it) {
        std::string tmp(it->c_str());
        cmd += tmp;
        cmd += " ";
    }
    return cmd;
}

void Environment::setGetoptEnv() {
    env["POSIXLY_CORRECT_BAK"] = env["POSIXLY_CORRECT"];
    env["POSIXLY_CORRECT"] = "YES";
}

void Environment::unsetGetoptEnv() {
    env["POSIXLY_CORRECT"] = env["POSIXLY_CORRECT_BAK"];
    env.erase("POSIXLY_CORRECT_BAK");
}

bool isBooted(boost::asio::io_context &io_context, const std::string &filename) {
    Argument arg(filename);
    boost::system::error_code error;
    tcp::socket socket(io_context);
    boost::asio::ip::address ip_address =
            boost::asio::ip::address::from_string("127.0.0.1", error);
    tcp::endpoint endpoint(ip_address, arg.conn.ports[arg.conn.hostNum]);
    socket.connect(endpoint, error);
    return !(error);
}
