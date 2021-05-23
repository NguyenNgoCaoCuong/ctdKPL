cmake_minimum_required(VERSION 3.17)
project(untitled3 C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled3 incompleted/symtab.c incompleted/main.c incompleted/debug.c incompleted/charcode.c incompleted/error.c incompleted/parser.c incompleted/reader.c incompleted/scanner.c incompleted/token.c incompleted/semantics.c)