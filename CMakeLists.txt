cmake_minimum_required(VERSION 3.10)
project(NanoSpark)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


file(GLOB_RECURSE SOURCES "src/*.cpp")
list(APPEND SOURCES main.cpp)

add_executable(NanoSpark ${SOURCES})

target_include_directories(NanoSpark PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_definitions(NanoSpark PRIVATE
    NANOSPARK_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)