cmake_minimum_required(VERSION 3.17)
project(messenger_server)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(Boost_INCLUDE_DIRS)

add_executable(ms_server main.cpp utils.cpp server.cpp server.h logging.cpp logging.h)
target_link_libraries(
        ms_server
        pthread
        boost_filesystem
        boost_system
)
