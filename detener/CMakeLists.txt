cmake_minimum_required(VERSION 3.12)
project(detener)

set(CMAKE_CXX_STANDARD 14)

include_directories(../commonSrc ../headers)

add_executable(detener
        detener.c
        ../headers/argValidator.h
        ../commonSrc/argValidator.c
        )

target_link_libraries(detener pthread)