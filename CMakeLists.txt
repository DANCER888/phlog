cmake_minimum_required(VERSION 3.30)
project(phlog)

set(CMAKE_CXX_STANDARD 23)

add_executable(phlog
        example/main.cpp
        include/phlog.h)
