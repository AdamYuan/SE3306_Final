cmake_minimum_required(VERSION 3.15)
project(GL3W)

add_library(gl3w STATIC src/gl3w.c)
target_include_directories(gl3w PUBLIC include)
