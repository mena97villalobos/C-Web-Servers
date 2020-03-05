cmake_minimum_required(VERSION 3.12)
project(projecto-2)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(projecto-2
        src/net.c
        src/net.h
        src/mime.c
        src/mime.h
        src/main.c
)

target_link_libraries(projecto-2 pthread)