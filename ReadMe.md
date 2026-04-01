## Custom High-Performance Memory Manager

### A specialized user-space memory allocator implemented in C++, designed for deterministic, low-latency performance in systems like matching engines and high-frequency trading (HFT) simulators.

### Design

#### This allocator uses a hybrid strategy to balance allocation speed and kernel overhead:

    - Segregated Free Lists (Quick Bins): Dedicated bins for 16, 32, 64, and 128-byte segments. This provides O(1) allocation/deallocation for small, common object sizes by bypassing the main heap search.
    - Arena Allocation: Utilizes mmap to fetch 4KB pages (Arenas) from the Linux kernel, minimizing the frequency of expensive system calls.
    - Coalescing & Splitting: Implements immediate neighbor coalescing on free() for larger fragments and a splitting strategy during alloc() to minimize external fragmentation in the main heap.
    - Cache Alignment: All allocations are 16-byte aligned to ensure optimal cache line utilization and prevent performance degradation on modern x86/ARM architectures.

### Performance & Benchmarking

#### The manager was profiled against glibc (malloc/free) over 100,000 operations.

|  Metric |  Result |
| ---- | ---- |
| Custom Alloc/DeAlloc Latency | 291 ns |
| Standard malloc/free | 42 ns |
| Test Environment | Intel i7-1065G7 running Ubuntu |
|    |    |

### Run it locally

This project is built for a Linux machine and requires a C++20 compatible compiler

1. From the project root, run these commands to generate, build and execute the app

    ```bash
    mkdir build
    cd build
    cmake --DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)

    ./mallocTestApp
    ```

This builds and executes the app. <br>
It runs a test with a basic allocation and deallocation script to ensure the system calls mmap and munmap work on the device. <br>
Following this check, the full test run testig 100,000 allocations and deallocations are performed. <ber>
This is then compared to the standard malloc/free from the glibc libraray, which is, of course, super optimized :\)
