cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
project(json-rpc-lib VERSION 0.0.1 LANGUAGES CXX)


option(COMPILE_TESTS "Enable tests" ON)
option(COMPILE_EXAMPLES "Enable examples" ON)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendor/
)

if (COMPILE_TESTS)
    add_subdirectory(tests)
endif ()


if (COMPILE_EXAMPLES)
    add_subdirectory(examples/simple)
endif ()
