cmake_minimum_required(VERSION 3.16)
project(atsc_calculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(atsc_calculator
    src/main.cpp

    src/model/atsc_config.h

    src/engine/atsc_tables.h
    src/engine/atsc_tables.cpp
    src/engine/calculator.h
    src/engine/calculator.cpp
    src/engine/cnr_tables.h
    src/engine/validation.h
    src/engine/validation.cpp

    src/io/config_file.h
    src/io/config_file.cpp
    src/ui/results_panel.h
    src/ui/results_panel.cpp

    src/ui/main_window.h
    src/ui/main_window.cpp
    src/ui/bootstrap_page.h
    src/ui/bootstrap_page.cpp
    src/ui/preamble_page.h
    src/ui/preamble_page.cpp
    src/ui/subframe_page.h
    src/ui/subframe_page.cpp
    src/ui/plp_page.h
    src/ui/plp_page.cpp
)

target_include_directories(atsc_calculator PRIVATE src)
target_link_libraries(atsc_calculator PRIVATE Qt6::Core Qt6::Widgets)
