cmake_minimum_required(VERSION 4.2)
project(RK1)

set(CMAKE_CXX_STANDARD 20)

add_executable(RK1 main.cpp
        task2_queue_semaphores.cpp)
add_executable(task1_pipeline task1_pipeline.cpp
        task2_queue_semaphores.cpp)
add_executable(task2_queue_semaphores task2_queue_semaphores.cpp
        task2_queue_semaphores.cpp)
