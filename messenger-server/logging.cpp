//
// Created by justanhduc on 20. 11. 18..
//

#include "logging.h"

#include <iostream>
#include <cstdarg>
#include <boost/process.hpp>
#include <chrono>
#include <ctime>

namespace fs = boost::filesystem;

inline int vscprintf (const char * format, va_list pargs) {
    /* from https://stackoverflow.com/a/4785411/4591601 */

    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(nullptr, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}

Logging::Logging() {
    std::string homedir = getenv("HOME");
    fs::path root(homedir);
    fs::path name(filename);
    fs::path p = root / name;

    ofs.open(p.string(), std::ofstream::out | std::ofstream::app);
    if (!ofs) {
        std::cerr << "Cannot open " << p.string() << " to write log" << std::endl;
        exit(-1);
    }
}

void Logging::logTime() {
    auto now = std::chrono::system_clock::now();
    auto timeNow = std::chrono::system_clock::to_time_t(now);
    ofs << std::ctime(&timeNow) << " ";
}

void Logging::log(const char * fmt, ...) {
    /* from https://stackoverflow.com/a/3280304/4591601 */

    va_list varptr;
    va_start(varptr, fmt);

    auto n = vscprintf(fmt, varptr);
    std::vector<char> buf(n);

    ::vsprintf(&buf[0], fmt, varptr);
    va_end(varptr);

    // writing log time
    logTime();

    // copy each character to the stream
    ofs << "\t";
    std::copy(buf.begin(), buf.end(), std::ostream_iterator<char>(ofs));
    ofs << std::endl;
}

void Logging::log(const strings& cmd, const std::string& delimiter) {
    logTime();
    ofs << "\t";
    for(auto & i : cmd) {
        ofs << i;
        ofs << delimiter;
    }
    ofs << std::endl;
}

void Logging::log(const std::string& cmd) {
    logTime();
    ofs << "\t" << cmd.c_str() << std::endl;
}
