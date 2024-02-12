cmake_minimum_required(VERSION 3.15)

project(HKGraphLib VERSION 0.0.1)

add_library(HKGraphLib STATIC)

target_include_directories(HKGraphLib
        INTERFACE
        $<TARGET_PROPERTY:HKGraphLib,INCLUDE_DIRECTORIES>)

target_sources(HKGraphLib
        PRIVATE
        Source/Graph.h
        Source/Graph.cpp
)
