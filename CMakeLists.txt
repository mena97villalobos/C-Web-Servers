cmake_minimum_required(VERSION 3.12)
project(projecto-2)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

add_executable(projecto-2
        src/net.c
        src/net.h
        src/server.c
        src/main.c
)

target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)
target_link_libraries(projecto-2 pthread)