cmake_minimum_required(VERSION 3.17)
project(messenger_server)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(Boost_INCLUDE_DIRS)

set(CMAKE_CUDA_COMPILER $ENV{CUDA_HOME}/bin/nvcc)
find_package(CUDA)
if (CUDA_FOUND)
    enable_language(CUDA)
    include_directories(
            $ENV{CUDA_HOME}/include
            ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
    )

    add_executable(ms_server main.cpp gpu.cpp utils.cpp server.cpp server.h logging.cpp logging.h)
    target_link_libraries(
            ms_server
            ${CUDA_LIBRARIES}
            boost_filesystem
            boost_system
    )
else ()
    add_executable(ms_server main.cpp utils.cpp gpu.cpp gpu.h server.cpp server.h logging.cpp logging.h)
    target_link_libraries(
            ms_server
            ${Boost_LIBRARIES}
            boost_filesystem
            boost_system
    )
endif ()
