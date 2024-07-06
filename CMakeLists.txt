cmake_minimum_required(VERSION 3.5)

project(QMG)

add_subdirectory(libQmageDecoder)

add_executable(QMG qmg.c)

target_link_libraries(QMG libQmageDecoder)