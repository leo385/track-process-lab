
#General building setup
cmake_minimum_required(VERSION 3.10)
project(TRACK_PROCESS_LAB)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
#set(CMAKE_C_COMPILER "gcc-11.1.0")
add_compile_options(-Wall -Wextra)


#Read all 'c' files from src
file(GLOB_RECURSE SOURCES src/*.c)

#exec settings
add_executable(main ${SOURCES})




