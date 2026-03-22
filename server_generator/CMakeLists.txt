cmake_minimum_required(VERSION 4.1)
project(server_generator)

set(CMAKE_CXX_STANDARD 14)

include_directories(inc)

add_executable(server_generator
        inc/dicionary_basic.h
        inc/dicionary_detail.h
        inc/functions.h
        inc/l1_interpreter.h
        inc/plp_struct.h
        inc/tcp_socket.h
        inc/timer.h
        src/functions/gen/generate_multi_frame.cpp
        src/functions/l1_calc.cpp
        src/functions/l1_interpreter.cpp
        src/functions/load_config_from_file.cpp
        src/functions/process_user_command.cpp
        src/functions/save_config_to_file.cpp
        src/main.cpp
        src/tcp_socket.cpp
        src/timer.cpp)
