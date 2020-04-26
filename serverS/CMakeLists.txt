cmake_minimum_required(VERSION 3.12)
project(server-secuencial)

set(CMAKE_CXX_STANDARD 14)

include_directories(../commonSrc ../headers)

add_executable(server-secuencial
        main.c
        ../commonSrc/commonHttpFunctions.c
        ../commonSrc/argValidator.c
        ../commonSrc/net.c
        ../commonSrc/mime.c
        ../headers/commonHttpFunctions.h
        ../headers/argValidator.h
        ../headers/mime.h
        ../headers/net.h
        )

target_link_libraries(server-secuencial pthread)