cmake_minimum_required(VERSION 3.15)
project(UnknownPower)

set(CMAKE_C_STANDARD 99)

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
        include/components/build_mode.h
        include/components/config_struct.h
        include/components/disassembly_struct.h
        include/components/dummy_stack.h
        include/components/ignore_opcode_struct.h
        include/components/instruction_range_struct.h
        include/components/instruction_struct.h
        include/components/output_mode.h
        include/components/registers_state_struct.h
        include/components/result_struct.h
        include/blacklist.h
        include/config.h
        include/general.h
        src/builder/builder.c
        src/builder/builder.h
        src/fault_handler/fault_handler.c
        src/fault_handler/fault_handler.h
        src/global/global.c
        src/global/global.h
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
        src/state_handler/state_handler.c
        src/state_handler/state_handler.h
        main.c)
