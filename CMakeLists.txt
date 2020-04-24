cmake_minimum_required(VERSION 3.15)
project(UnknownPower)

set(CMAKE_CXX_STANDARD 14)

SET(COMPILE_FLAGS "-Wno-write-strings -Wall")
SET(LINK_FLAGS    "-O3 -Wall -no-pie -pthread")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")

add_definitions(-DINTEL=0 -DPOWER_PC=1)
add_definitions(-DPROCESSOR=INTEL)

add_definitions(-DUSE_CAPSTONE=false)

include_directories(.)
include_directories(include)
include_directories(include/components)
include_directories(src)
include_directories(src/InstructionManager)
include_directories(src/fault_handler)
include_directories(src/global)
include_directories(src/helpers)
include_directories(src/injector)
include_directories(src/state_handler)

file(GLOB_RECURSE UnknownPowerSource
        ${PROJECT_SOURCE_DIR}/include/*.hpp
        ${PROJECT_SOURCE_DIR}/src/*.hpp
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.c
        )

# target_link_libraries(UnknownPower capstone)
add_executable(UnknownPower ${UnknownPowerSource})
