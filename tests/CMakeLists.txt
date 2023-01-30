

project(tests)

find_package(Threads)

add_executable(${PROJECT_NAME} 
    main.cpp
    id.cpp
    error.cpp
    helper_functions.cpp
    request.cpp
    response.cpp
    server_base.cpp
)

target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

target_link_libraries(${PROJECT_NAME} json-rpc-lib Threads::Threads)