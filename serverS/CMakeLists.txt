cmake_minimum_required(VERSION 3.12)
project(server-secuencial)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(server-secuencial
        src/argValidator.c
        src/net.c
        src/mime.c
        src/main.c
        headers/argValidator.h
        headers/mime.h
        headers/net.h
        )

target_link_libraries(server-secuencial pthread)