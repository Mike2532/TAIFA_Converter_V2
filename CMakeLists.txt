cmake_minimum_required(VERSION 4.0)
project(TAIFA_Converter_V2)

set(CMAKE_CXX_STANDARD 20)

add_executable(TAIFA_Converter_V2 main.cpp
        src/MealyHandler.h
        src/MoorHandler.h)
