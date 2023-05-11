cmake_minimum_required(VERSION 3.24)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(untitled main.cpp "src/XModemSender  .cpp" src/XModemReceiver.cpp "src/SerialPort .cpp" include/XModemReceiver.h include/XModemSender.h include/variables.h)
