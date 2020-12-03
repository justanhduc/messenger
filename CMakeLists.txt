cmake_minimum_required(VERSION 3.17)
project(messenger_server)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(Boost_INCLUDE_DIRS)

set(CMAKE_CUDA_COMPILER $ENV{CUDA_HOME}/bin/nvcc)
enable_language(CUDA)
include_directories(
        $ENV{CUDA_HOME}/include
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)
link_directories($ENV{CUDA_HOME}/lib64/stubs)

add_executable(ms_server main.cpp gpu.cpp utils.cpp server.cpp server.h logging.cpp logging.h)
target_link_libraries(
        ms_server
        nvidia-ml
        pthread
        boost_filesystem
        boost_system
)
