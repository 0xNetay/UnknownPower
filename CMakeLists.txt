cmake_minimum_required(VERSION 3.15)
project(UnknownPower)

set(CMAKE_CXX_STANDARD 14)

# SET(COMPILE_FLAGS "$< -O3 -Wall -no-pie -l:libcapstone.a -o $@ -pthread")
# SET(LINK_FLAGS    "-c $< -o $@ -Wall")

# SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS}")
# SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")

include_directories(.)
include_directories(include)
include_directories(include/components)
include_directories(src)
include_directories(src/builder)
include_directories(src/fault_handler)
include_directories(src/global)
include_directories(src/helpers)
include_directories(src/injector)
include_directories(src/state_handler)

add_executable(UnknownPower
        src/low_level/fault_handler/fault_handler.c
        src/global/global_blacklist.c
        src/global/global_config.c
        src/global/global_variables.c
        src/global/global_variables_linux.c
        src/global/output.c
        src/global/output.h
        src/global/preamble.c
        src/helpers/helpers.c
        src/helpers/helpers.h
        src/injector/injector.c
        src/injector/injector.h
        src/low_level/state_handler/state_handler.c
        src/low_level/state_handler/state_handler.h
        main.cpp src/builder/Definitions.hpp src/builder/Builder.cpp src/builder/Builder.hpp src/config/Definitions.hpp src/config/ConfigManager.hpp)
