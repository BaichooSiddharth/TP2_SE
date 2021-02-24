cmake_minimum_required(VERSION 3.11)
project(TP2 C)

set(CMAKE_C_STANDARD 99)

add_executable(TP2 main.c main.h blocking_q.c blocking_q.h task.c task.h)

find_package(Threads REQUIRED)
target_link_libraries(TP2 PRIVATE Threads::Threads)
