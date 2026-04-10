cmake_minimum_required(VERSION 3.10)
project(server_generator)

set(CMAKE_CXX_STANDARD 14)

include_directories(inc)

add_executable(server_generator
        inc/atsc_server.h
        inc/atsc_config.h
        inc/binary_utils.h
        inc/l1_calculator.h
        inc/l1_generator.h
        inc/l1_interpreter_class.h
        inc/dicionary_basic.h
        inc/dicionary_detail.h
        inc/plp_struct.h
        inc/tcp_socket.h
        inc/timer.h
        src/atsc_server.cpp
        src/atsc_config.cpp
        src/binary_utils.cpp
        src/l1_calculator.cpp
        src/l1_generator.cpp
        src/l1_interpreter_class.cpp
        src/main.cpp
        src/tcp_socket.cpp
        src/timer.cpp)
