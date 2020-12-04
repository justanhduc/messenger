//
// Created by justanhduc on 20. 11. 18..
//

#ifndef MESSENGER_SERVER_LOGGING_H
#define MESSENGER_SERVER_LOGGING_H

#include <string>
#include <fstream>
#include <vector>

typedef std::vector<std::string> strings;

enum LoggingLevel {
    INFO,
    WARNING,
    ERROR
};

class BaseLogger {
protected:
    std::string filename = ".messenger_server.log";
    std::ofstream ofs;
    void logTime();

    void log(const char *, ...);

    void log(const strings&, const std::string& delimiter=" ");

    void log(const std::string &str);

public:
    BaseLogger();

    ~BaseLogger() {
        ofs.close();
    }

};

class Logger : BaseLogger {
public:
    template<typename... Args>
    explicit Logger(Args &&... args) : BaseLogger(std::forward<Args>(args)...) {};

    template<typename... Args>
    void log(LoggingLevel level, const std::string& file, const int line, Args &&... args) {
        std::string levelStr;
        switch (level) {
            case INFO:
                levelStr = "INFO";
                break;
            case WARNING:
                levelStr = "WARNING";
                break;
            case ERROR:
                levelStr = "ERROR";
                break;
        }

        // log time
        logTime();

        // write some metadata first
        ofs << "\t";
        ofs << levelStr << ": ";

        // write log contents here
        BaseLogger::log(std::forward<Args>(args)...);

        // where
        ofs << std::endl;
        ofs << "\t" << "In " << file << " line " << line << std::endl;

        if (level == ERROR)
            exit(-1);
    };
};

#endif //MESSENGER_SERVER_LOGGING_H
