cmake_minimum_required(VERSION 3.10)
 
project(emu4380)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(emu4380 src/main.cpp src/emu4380.cpp)
target_include_directories(emu4380 PRIVATE include)
 
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
 
add_executable(runTests test/tests.cpp src/emu4380.cpp)
target_include_directories(runTests PRIVATE include ${GTEST_INCLUDE_DIRS})
target_link_libraries(runTests ${GTEST_LIBRARIES} Threads::Threads)
 
