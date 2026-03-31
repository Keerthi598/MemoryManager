#include <iostream>
#include <chrono>
#include <vector>

#include "HeapManager.h"

void runStress() {
    // std::cout << 
    std::cout << "\n--- Run Stress Test ---" << std::endl;

    const int iterations = 100000;
    std::vector<void*> ptrs;
    ptrs.reserve(iterations);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        ptrs.push_back(HeapManager::alloc((i % 128) + 8));
        if (i % 2 == 0) {
            HeapManager::free(ptrs.back());
            ptrs.pop_back();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "\n--- Performance Results (Custom) ---" << std::endl;
    std::cout << "\n--- Number of total allocs + deallics = " << iterations << " ---" << std::endl;
    std::cout << "Total Time: " << diff.count() / 1e6 << " ms" << std::endl;
    std::cout << "Avg Latency per Op: " << diff.count() / iterations << " ns" << std::endl;

    ptrs.clear();

    // Try malloc and free

    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        ptrs.push_back(malloc((i % 128) + 8));
        if (i % 2 == 0) {
            free(ptrs.back());
            ptrs.pop_back();
        }
    }

    end = std::chrono::high_resolution_clock::now();
    diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "\n--- Performance Results (Malloc and Free) ---" << std::endl;
    std::cout << "\n--- Number of total allocs + deallics = " << iterations << " ---" << std::endl;
    std::cout << "Total Time: " << diff.count() / 1e6 << " ms" << std::endl;
    std::cout << "Avg Latency per Op: " << diff.count() / iterations << " ns" << std::endl;
    std::cout << "\n" << std::endl;
    std::cout << "--- Stress Test Complete ---" << std::endl;
    std::cout << "\n\n\n" << std::endl;
}

void testAllocator() {
    std::cout << "--- Starting Memory Manager Test ---" << std::endl;

    // 1. Test Basic Allocation and splitting
    void* p1 = HeapManager::alloc(100);
    void* p2 = HeapManager::alloc(200);
    std::cout << "[CHECK]   Allocated p1 (100b) and p2 (200b)" << std::endl;
    std::cout << "[STATUS]  p1 address: " << p1 << std::endl;
    std::cout << "[STATUS]  p2 address: " << p2 << std::endl;

    // 2. Test Coalescing
    std::cout << "[ACTION]  Freeing p1 and p2 to trigger merge..." << std::endl;
    HeapManager::free(p1);
    HeapManager::free(p2);

    // 3. Test Reallocation
    void* p3 = HeapManager::alloc(350);
    std::cout << "[CHECK]   Allocated p3(350b) after merge" << std::endl;
    std::cout << "[STATUS]  p3 address: " << p3 << std::endl;

    HeapManager::free(p3);
    std::cout << "--- Test Complete ---" << std::endl;
}

int main() {
    testAllocator();
    std::cout << "--- --- ---" << std::endl;
    runStress();

    return 0;
}