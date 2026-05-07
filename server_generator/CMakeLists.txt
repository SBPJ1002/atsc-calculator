cmake_minimum_required(VERSION 3.14)
project(server_generator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(inc)

find_package(Threads REQUIRED)

option(BUILD_TESTS "Build the test suite" ON)

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
        inc/atsc_tables.h
        inc/json_handler.h
        src/atsc_server.cpp
        src/atsc_config.cpp
        src/binary_utils.cpp
        src/l1_calculator.cpp
        src/l1_generator.cpp
        src/l1_interpreter_class.cpp
        src/json_handler.cpp
        src/main.cpp
        src/tcp_socket.cpp
        src/timer.cpp)

target_link_libraries(server_generator Threads::Threads)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
