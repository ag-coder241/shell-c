cmake_minimum_required(VERSION 3.13)

project(codecrafters-shell C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(shell
    src/main.c
    src/parser.c
    src/executor.c
    src/completion.c
    src/builtins.c
    src/redirection.c
)

target_include_directories(shell PRIVATE include)

target_link_libraries(shell PRIVATE readline)

