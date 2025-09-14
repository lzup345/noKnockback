cmake_minimum_required(VERSION 3.22.1)

project("test")

add_library(test SHARED
        efmod.cpp
)


target_include_directories(test
        PUBLIC
        ./efmod-api
        ./tefmod-api
)