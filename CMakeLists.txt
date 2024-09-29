cmake_minimum_required(Version 3.20.0)
project(weather_cli)

add_executable(${PROJECT_NAME} main.c cJSON.c terminal_support.c image_to_ascii.c)


# This would actually install the programm to /usr/bin

# install(TARGETS ${PROJECT_NAME} DESTINATON bin)
