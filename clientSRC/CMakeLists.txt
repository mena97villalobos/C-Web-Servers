cmake_minimum_required(VERSION 3.12)
project(projecto-client)

set(CMAKE_CXX_STANDARD 14)

include_directories(clientSRC)

add_executable(projecto-client
        client.c
        ../headers/argValidator.h
        ../commonSrc/argValidator.c
        )

target_link_libraries(projecto-client pthread)