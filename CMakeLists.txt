cmake_minimum_required(VERSION 3.3)
project(proxy_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
        raii_file_descriptor.cpp raii_file_descriptor.h
        throw_error.cpp throw_error.h
        ipv4_endpoint.cpp ipv4_endpoint.h
        sockets.cpp sockets.h
        epoll_wrapper.cpp epoll_wrapper.h
        proxy_server.cpp proxy_server.h
        event_handler.cpp event_handler.h DNS_resolver.cpp DNS_resolver.h)
add_executable(proxy_server ${SOURCE_FILES})