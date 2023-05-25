cmake_minimum_required(VERSION 3.25)
project(TP_AAIA_LS C)

set(CMAKE_C_STANDARD 11)

add_executable(TP_AAIA_LS main.c)
target_link_libraries(TP_AAIA_LS m)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")
