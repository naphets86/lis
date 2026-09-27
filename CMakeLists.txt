cmake_minimum_required(VERSION 3.31)
project(LISInfinity C)

# C-Standard und Compiler-Flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -g -pedantic")

# Output-Verzeichnis
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Zielgeräte (Executables)
add_executable(lis_scheduler LISInfinityScheduler.c)
add_executable(lis_extended LISIfinity.c)
add_executable(lis_verification LISInfinityVerification.c)

# Math-Library linken
target_link_libraries(lis_scheduler m)
target_link_libraries(lis_extended m)
target_link_libraries(lis_verification m)