/* Created by justanhduc on 11/6/20. */

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <sys/socket.h>

#include "utils.h"

Connection::Connection(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        logger.log(ERROR, __FILE__, __LINE__, "Cannot open host file");
    else {
        file >> host >> port;
    }
}

void Environment::setEnv() {
    auto itFlag = envFlags.begin();
    auto itVal = envFlagVals.begin();
    for (itFlag = envFlags.begin(), itVal = envFlagVals.begin();
         (itFlag != envFlags.end()) && (itVal != envFlagVals.end());
         ++itFlag, ++itVal)
        env[*itFlag] = *itVal;
}

void Environment::parseFlag(std::string flag) {
    strings envs = splitString(std::move(flag), ";");
    for (auto &it: envs) {
        strings flagsVals = splitString(std::move(it), "=");
        envFlags.push_back(flagsVals[0]);
        envFlagVals.push_back(flagsVals[1]);
    }
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
    /* Parse options */
    while (true) {
        c = getopt_long(argc, cargv.data(), "H:", longOptions, &optionIdx);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                if (strcmp(longOptions[optionIdx].name, "cd") == 0) {
                    currentDir = optarg;
                    cmdIdx += 2;
                } else if (strcmp(longOptions[optionIdx].name, "env") == 0) {
                    env.parseFlag(optarg);
                    cmdIdx += 2;
                } else if (strcmp(longOptions[optionIdx].name, "auto_server") == 0) {
                    cmdIdx += 1;
                } else if (strcmp(longOptions[optionIdx].name, "exclude") == 0 ||
                           strcmp(longOptions[optionIdx].name, "sync") == 0 ||
                           strcmp(longOptions[optionIdx].name, "host") == 0 ||
                           strcmp(longOptions[optionIdx].name, "sync_dest") == 0 ||
                           strcmp(longOptions[optionIdx].name, "include") == 0 ||
                           strcmp(longOptions[optionIdx].name, "ln") == 0 ||
                           strcmp(longOptions[optionIdx].name, "ln_dest") == 0) {
                    cmdIdx += 2;
                } else if (strcmp(longOptions[optionIdx].name, "kill") == 0) {
                    action = KILL_SERVER;
                }
                break;
            case 'H':
                cmdIdx += 2;
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
        logger.log(ERROR, __FILE__, __LINE__, "Cannot send package");

    res = send(fd, string.c_str(), string.size(), MSG_CONFIRM);
    if (res < 0)
        logger.log(ERROR, __FILE__, __LINE__, "Cannot send package");
}

std::string recvString(int fd) {
    int res;
    uint32_t size;
    res = recv(fd, &size, sizeof(uint32_t), 0);
    if (res == -1)
        logger.log(ERROR, __FILE__, __LINE__, "Cannot receive package");
    size = ntohl(size);

    std::vector<char> rcvBuf;              // Allocate a receive buffer
    rcvBuf.resize(size, 0x00);             // with the necessary size
    res = recv(fd, &(rcvBuf[0]), size, 0); // Receive the string data
    if (res == -1)
        logger.log(ERROR, __FILE__, __LINE__, "Cannot receive package");

    std::string string;
    string.assign(&(rcvBuf[0]), rcvBuf.size());
    return string;
}

std::string buildTsCommand(strings &argv, int index) {
    std::string cmd = "ts ";
    for (auto it = argv.begin() + index; it != argv.end(); ++it) {
        std::string tmp(it->c_str()); // truncate \00 in it
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

bool isBooted(boost::asio::io_context &io_context,
              const std::string &filename) {
    Argument arg(filename);
    boost::system::error_code error;
    tcp::socket socket(io_context);
    boost::asio::ip::address ip_address =
            boost::asio::ip::address::from_string("127.0.0.1", error);
    tcp::endpoint endpoint(ip_address, arg.conn.port);
    socket.connect(endpoint, error);
    return !(error);
}
