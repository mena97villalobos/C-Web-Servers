cmake_minimum_required(VERSION 3.12)
project(server-threads)

set(CMAKE_CXX_STANDARD 14)

include_directories(../commonSrc ../headers)

add_executable(server-threads
        main.c
        ../commonSrc/argValidator.c
        ../commonSrc/net.c
        ../commonSrc/mime.c
        ../headers/argValidator.h
        ../headers/mime.h
        ../headers/net.h
        )

target_link_libraries(server-threads pthread)