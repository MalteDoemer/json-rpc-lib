
project(simple)

find_package(Threads)

add_executable(${PROJECT_NAME} 
    main.cpp
)

target_compile_options(${PROJECT_NAME} PUBLIC)

target_link_libraries(${PROJECT_NAME} json-rpc-lib Threads::Threads)