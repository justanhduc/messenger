//
// Created by justanhduc on 20. 11. 18..
//

#ifndef MESSENGER_SERVER_LOGGING_H
#define MESSENGER_SERVER_LOGGING_H

#include <string>
#include <fstream>
#include <vector>

typedef std::vector<std::string> strings;

class Logging {
private:
    std::string filename = ".messenger_server.log";
    std::ofstream ofs;
    void logTime();

public:
    Logging();

    void log(const char *, ...);

    void log(const strings&, const std::string& delimiter=" ");

    void log(const std::string &cmd);

    ~Logging() {
        ofs.close();
    }

};

#endif //MESSENGER_SERVER_LOGGING_H
