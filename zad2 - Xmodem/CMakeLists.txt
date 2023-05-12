cmake_minimum_required(VERSION 3.24)
project(XModem)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(untitled main.cpp
        src/SerialPort.cpp
        src/CalculateCheckSum.cpp
        include/variables.h
        include/SerialPort.h
        include/CalculateCheckSum.h)