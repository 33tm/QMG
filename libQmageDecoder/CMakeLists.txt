cmake_minimum_required(VERSION 3.5)

file(GLOB SRC *.c *.h)

add_library(libQmageDecoder SHARED ${SRC})