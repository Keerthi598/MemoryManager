cmake_minimum_required(VERSION 3.12)
project(HeapManager VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/O2 /Ot /Oi /Gy) # MSVC Release optimizations
else()
    add_compile_options(-O3 -march=native -flto) # GCC/Clang: Max speed + Link Time Optimization
endif()

add_subdirectory(HeapManagerLib)

add_executable(mallocTestApp main.cpp)

target_link_libraries(
    mallocTestApp
    PUBLIC
        heap_manager
)

