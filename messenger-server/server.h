//
// Created by justanhduc on 20. 11. 18..
//

#ifndef MESSENGER_SERVER_SERVER_H
#define MESSENGER_SERVER_SERVER_H

#include "utils.h"

class MessengerServer {
public:
    Argument arg;
    int pid = -1;

    MessengerServer(boost::asio::io_context &ioContext, const std::string &filename) :
            ioContext(ioContext),
            signal_(ioContext, SIGCHLD),
            acceptor(ioContext),
            socket_(ioContext) {
        arg = Argument(filename);
        int option = 1;
        struct timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        acceptor.open(tcp::v4());
        if (setsockopt(
                acceptor.native_handle(), SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) < 0)
            logger.log(ERROR, __FILE__, __LINE__, "setsockopt failed");

        acceptor.bind(tcp::endpoint(tcp::v4(), arg.conn.port));
        acceptor.listen(0);

        startSignalWait();
        startAccept();
    }

    ~MessengerServer() {
        socket_.close();
    }

private:

    void startSignalWait() {
        signal_.async_wait(boost::bind(&MessengerServer::handle_signal_wait, this));
    }

    void handle_signal_wait() {
        if (acceptor.is_open()) {
            // Reap completed child processes so that we don't end up with zombies.
            int status = 0;
            while (waitpid(-1, &status, WNOHANG) > 0) {}

            startSignalWait();
        }
    }

    void startAccept() {
        acceptor.async_accept(socket_, boost::bind(&MessengerServer::handleAccept, this, _1));
    }

    void handleAccept(const boost::system::error_code &ec) {
        if (!ec) {
            ioContext.notify_fork(boost::asio::io_context::fork_prepare);

            if (fork() == 0) {
                ioContext.notify_fork(boost::asio::io_context::fork_child);
                acceptor.close();
                signal_.cancel();
                startProcess();
            } else {
                ioContext.notify_fork(boost::asio::io_context::fork_parent);

                socket_.close();

                /* TODO: cannot kill the server yet */
                if (arg.action == KILL_SERVER)
                    kill(pid, SIGKILL);

                startAccept();
            }
        } else {
            logger.log(WARNING, __FILE__, __LINE__, "Accept error: %s", ec.message().c_str());
            startAccept();
        }
    }

    void startProcess();

    boost::asio::io_context &ioContext;
    boost::asio::signal_set signal_;
    boost::system::error_code error;
    tcp::acceptor acceptor;
    tcp::socket socket_;
};


#endif //MESSENGER_SERVER_SERVER_H
