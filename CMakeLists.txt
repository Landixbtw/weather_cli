cmake_minimum_required(VERSION 3.10)
project(weather_cli)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set build type if not set
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CACA REQUIRED caca)

# Add executable
add_executable(weather_cli 
    src/main.c 
    src/terminal_support.c 
    src/image_to_ascii.c 
    src/cJSON.c
)

# Include directories
target_include_directories(weather_cli PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources
    ${CURL_INCLUDE_DIRS}
    ${CACA_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(weather_cli PRIVATE 
    ${CURL_LIBRARIES}
    ${CACA_LIBRARIES}
)

# Copy resources
file(COPY src/resources DESTINATION ${CMAKE_BINARY_DIR})
