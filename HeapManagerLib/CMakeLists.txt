add_library(heap_manager HeapManager.cpp)

target_include_directories(
    heap_manager
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)