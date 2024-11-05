cmake_minimum_required(VERSION 3.10)

project(QMG)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES src/*)

include_directories(src)

add_executable(QMG ${SOURCES})