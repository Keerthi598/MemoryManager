#ifndef CUSTOM_HEAP_MANAGER_H
#define CUSTOM_HEAP_MANAGER_H

#include <memory>

class HeapManager {
public:
    static void* alloc(size_t size);
    static void free(void* addr);

    static void heapDump();

    HeapManager() {
        pages = nullptr;
    }

    ~HeapManager() {
        CleanUp();
    }

private:
    static const size_t ARENA_SIZE = 4096;

    struct BlockHeader {
        size_t size;
        BlockHeader* next{nullptr};
        BlockHeader* prev{nullptr};
        bool isFree{true};
    };

    struct Arena {
        // size_t size{4096};
        Arena* nextArena{nullptr};

        // size_t ARENA_AVAIL_SIZE = ARENA_SIZE - sizeof(Arena*);

        BlockHeader* head{nullptr};
    };

    static Arena* pages;

    static inline BlockHeader* quickBins[4] = {nullptr, nullptr, nullptr, nullptr}; 

    static int getBinIndex(size_t size) {
        if (size <= 16) return 0;
        if (size <= 32) return 1;
        if (size <= 64) return 2;
        if (size <= 128) return 3;
        return -1; // Too big for quick-bin
    }

    static void* GetNewVMPageFromKernel(int units);
    static void CleanUp();

    static void SplitBlock(HeapManager::BlockHeader* block, size_t size);
};

#endif // CUSTOM_HEAP_MANAGER_H