cmake_minimum_required(VERSION 3.29)
project(weather_cli C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(weather_cli
    cJSON.c
    cJSON.h
    main.c)
