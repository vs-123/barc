cmake_minimum_required(VERSION 3.10)

project(barc)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRCS "src/main.c")

add_executable(barc ${SRCS})
target_compile_options(barc PRIVATE -Wall -Wextra -Wpedantic)
