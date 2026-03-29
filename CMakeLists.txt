cmake_minimum_required(VERSION 4.2)
project(RK1)

set(CMAKE_CXX_STANDARD 20)

add_executable(RK1 main.cpp)
add_executable(task1_warehouse_sync task1_warehouse_sync.cpp)
add_executable(task2_production_semaphores task2_production_semaphores.cpp)
