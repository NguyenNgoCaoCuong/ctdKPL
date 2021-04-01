cmake_minimum_required(VERSION 3.17)
project(untitled3 C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled3 incompleted/scanner.c incompleted/charcode.c incompleted/error.c incompleted/reader.c incompleted/token.c)