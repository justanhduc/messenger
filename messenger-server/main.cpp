/* Created by justanhduc on 11/5/20. */

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <sys/stat.h>

#include "server.h"
#include "utils.h"

std::string tmpRoot = "/tmp/messenger-tmp";
Logger logger;

bool isPathExist(const std::string &s) {
    struct stat buffer{};
    return (stat(s.c_str(), &buffer) == 0);
}

int main() {
    if (!isPathExist(tmpRoot))
        mkdir(tmpRoot.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::string root = std::getenv("HOME");
    std::string filename = "/.hosts_ports";
    boost::asio::io_context ioContext;
    if (isBooted(ioContext, root + filename)) {
        auto str = "Server is already running...";
        logger.log(INFO, __FILE__, __LINE__, str);
        std::cout << str << std::endl;
        return 0;
    }

    try {
        MessengerServer server(ioContext, root + filename);
        int p[2], res;
        res = pipe(p);
        if (res == -1)
            logger.log(ERROR, __FILE__, __LINE__, "Cannot open a pipe");

        if (fork() == 0) {
            close(p[0]);
            close(0);
            close(1);
            close(2);
            auto pid = ::getpid();
            res = write(p[1], &pid, sizeof(int));
            if (res == -1)
                logger.log(ERROR, __FILE__, __LINE__, "Cannot write to pipe");

            setsid();
            ioContext.run();
            exit(0);
        } else {
            close(p[1]);
            res = read(p[0], &server.pid, sizeof(int));
            if (res == -1)
                logger.log(ERROR, __FILE__, __LINE__, "Cannot write to pipe");

            auto str = "Server has been booted";
            std::cout << str << std::endl;
            logger.log(INFO, __FILE__, __LINE__, str);
        }
    } catch (std::exception &e) {
        logger.log(WARNING, __FILE__, __LINE__, "%s", e.what());
    }

    return 0;
}