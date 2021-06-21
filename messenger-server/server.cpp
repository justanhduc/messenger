//
// Created by justanhduc on 20. 11. 18..
//

#include "server.h"
#include "gpu.h"

void MessengerServer::startProcess() {
    strings argv;
    int argc;

    // start reading in arguments
    socket_.read_some(boost::asio::buffer(&argc, sizeof(int)), error);
    if (error)
        logger.log(ERROR, __FILE__, __LINE__, error.message()); // Some other error.
    argc = ntohl(argc);

    for (int i = 0; i < argc; ++i) {
        int len;
        size_t res =
                socket_.read_some(boost::asio::buffer(&len, sizeof(int)), error);
        if (res < 0)
            logger.log(ERROR, __FILE__, __LINE__, "Error reading from client");
        len = ntohl(len);

        boost::array<char, BSIZE> buf{};
        res = socket_.read_some(boost::asio::buffer(buf, len), error);
        if (res < 0)
            logger.log(ERROR, __FILE__, __LINE__, "Error reading from client");

        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            logger.log(ERROR, __FILE__, __LINE__,
                       error.message()); // Some other error.

        std::string argument(buf.begin(), len);
        argv.push_back(argument);
    }
    logger.log(INFO, __FILE__, __LINE__, argv);

    // set up to execute commands
    arg.env.setGetoptEnv();
    auto optIdx = arg.parseOpts(argc, argv);
    arg.env.unsetGetoptEnv();

    switch (arg.action) {
        case KILL_SERVER:
            exit(1);
        case SHOW_FREE_GPUS:
            dup2(socket_.native_handle(), STDOUT_FILENO);
            dup2(socket_.native_handle(), STDERR_FILENO);
            showGpuInfo();
            socket_.close();
            break;
        case SHOW_GPUS:
            dup2(socket_.native_handle(), STDOUT_FILENO);
            dup2(socket_.native_handle(), STDERR_FILENO);
            showGpuInfo(false);
            socket_.close();
            break;
        case COUNT_FREE_GPUS: {
            auto gpuList = getFreeGpuList();
            auto numGpus = gpuList.size();
            dup2(socket_.native_handle(), STDOUT_FILENO);
            dup2(socket_.native_handle(), STDERR_FILENO);
            socket_.close();
            std::cout << numGpus << std::endl;
            break;
        }
        case TASK_SPOOLER: {
            auto tsCmd = buildTsCommand(argv, optIdx);
            arg.env.setEnv();
            logger.log(INFO, __FILE__, __LINE__, tsCmd);

            ioContext.notify_fork(boost::asio::io_context::fork_child);
            acceptor.close();
            if (true) {
                int result = bp::system(tsCmd, fd.bind(1, socket_.native_handle()),
                                        fd.bind(2, socket_.native_handle()),
                                        bp::start_dir(arg.currentDir), arg.env.env);

                /* another option is using dup2 and execvp
                 * however, execvp exits so any code after that
                 * will not be executed */
                //                dup2(socket_.native_handle(), STDOUT_FILENO);
                //                dup2(socket_.native_handle(), STDERR_FILENO);
                //                close(socket_.native_handle());
                //                execvp(tsCmd[0], tsCmd.data());
                if (result != 0)
                    logger.log(ERROR, __FILE__, __LINE__,
                               "Command failed to execute. Exit code: %d", result);
            } else {
                // running a job causes the client to wait.
                // rather lets queue a job without stdout and std_err,
                // and then report the number back.
                int result = bp::system(tsCmd, bp::std_out.close(),
                                        fd.bind(2, socket_.native_handle()));
                if (result != 0)
                    std::cerr << "Command failed to execute" << std::endl;

                result = bp::system("ts -q", fd.bind(1, socket_.native_handle()),
                                    fd.bind(2, socket_.native_handle()));
                if (result != 0)
                    std::cerr << "Command failed to execute" << std::endl;
            }

            socket_.close();
            break;
        }
        default:
            logger.log(ERROR, __FILE__, __LINE__, "Wrong action %d!", arg.action);
    }
}
