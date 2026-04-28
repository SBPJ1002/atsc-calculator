include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(atsc_tests
    test_e2e_atsc.cpp
    ${CMAKE_SOURCE_DIR}/src/atsc_config.cpp
    ${CMAKE_SOURCE_DIR}/src/binary_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/l1_calculator.cpp
    ${CMAKE_SOURCE_DIR}/src/l1_generator.cpp
    ${CMAKE_SOURCE_DIR}/src/l1_interpreter_class.cpp
    ${CMAKE_SOURCE_DIR}/src/json_handler.cpp
)

target_include_directories(atsc_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(atsc_tests PRIVATE
    Catch2::Catch2WithMain
    Threads::Threads
)

# Working dir isolado: setConfig regenera config/log/* — nao mexe no config/
# do projeto.
set(TEST_RUNTIME_DIR ${CMAKE_BINARY_DIR}/test_runtime)
file(MAKE_DIRECTORY ${TEST_RUNTIME_DIR}/config/log)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(atsc_tests
    WORKING_DIRECTORY ${TEST_RUNTIME_DIR}
)
