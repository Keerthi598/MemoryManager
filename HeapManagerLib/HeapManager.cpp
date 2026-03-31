#include "HeapManager.h"
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <vector>

HeapManager::Arena* HeapManager::pages = nullptr;

void *HeapManager::alloc(size_t size)
{
    // Align size_t to improve cache line efficiency
    size = (size + 15) & ~15;


    // Try to quick match to a precached bin if the size in small enough
    int bin = getBinIndex(size);

    if (bin != -1 && quickBins[bin]) {
        BlockHeader* preCachedBlock = quickBins[bin];
        quickBins[bin] = preCachedBlock->next;
        preCachedBlock->isFree = false;
        preCachedBlock->next = nullptr;
        preCachedBlock->prev = nullptr;
        void* retPointer = (void*)(reinterpret_cast<char*>(preCachedBlock) + sizeof(BlockHeader));
        
        return retPointer;
    }

    // We could not find a suitable block in the quick bin cache

    // Find an existing block big enough for the current request
    size_t minimunSizeNeeded = size + sizeof(BlockHeader) + 16; // At least 16 bytes of data extra needed if we need to split

    Arena* currentArena = pages;

    while (currentArena != nullptr) {
        BlockHeader* currentBlock = currentArena->head->next;

        while (currentBlock != nullptr) {

            if (currentBlock->size == size && currentBlock->isFree) {
                // Perfect Match Found
                currentBlock->isFree = false;

                return (void*)(reinterpret_cast<char*>(currentBlock) + sizeof(BlockHeader));
            }

            if (currentBlock->size >= minimunSizeNeeded && currentBlock->isFree) {
                // Split the block
                // A big enough block is found
                SplitBlock(currentBlock, size);
                return (void*)(reinterpret_cast<char*>(currentBlock) + sizeof(BlockHeader));
            }

            currentBlock = currentBlock->next;
        }
        
        currentArena = currentArena->nextArena;
    }

    // If no block big enough, ask kernel for more memory space
    // Get a new Arena
    Arena* newArena = (Arena*)GetNewVMPageFromKernel(ARENA_SIZE);
    newArena->nextArena = pages;
    pages = newArena;
    
    // The new block created right after the arena space
    BlockHeader* headBlock = (BlockHeader*)(reinterpret_cast<char*>(newArena) + sizeof(Arena));
    headBlock->isFree = false;
    headBlock->prev = nullptr;

    BlockHeader* newBlock = (BlockHeader*)(reinterpret_cast<char*>(headBlock) + sizeof(BlockHeader));
    newBlock->isFree = true;
    newBlock->next = nullptr;
    newBlock->prev = headBlock;
    newBlock->size = ARENA_SIZE - sizeof(Arena) - sizeof(BlockHeader) - sizeof(BlockHeader);

    newArena->head = headBlock;
    headBlock->next = newBlock;
    void* retPointer = reinterpret_cast<char*>(newBlock) + sizeof(BlockHeader);
    SplitBlock(newBlock, size);

    return retPointer;
}

void HeapManager::SplitBlock(HeapManager::BlockHeader* block, size_t size)
{
    BlockHeader* newBlock = (BlockHeader*)(reinterpret_cast<char*>(block) + size + sizeof(BlockHeader));
    newBlock->next = block->next;
    newBlock->prev = block;
    newBlock->isFree = true;
    newBlock->size = block->size - size - sizeof(BlockHeader);

    if (newBlock->next)
        newBlock->next->prev = newBlock;

    block->size = size;
    block->next = newBlock;
    block->isFree = false;
}

void HeapManager::free(void* addr)
{
    BlockHeader* block = (BlockHeader*)(static_cast<char*>(addr) - sizeof(BlockHeader));
    
    int bin = getBinIndex(block->size);

    if (bin != -1) {
        if (block->prev) 
            block->prev->next = block->next;
        if (block->next)
            block->next->prev = block->prev;

        block->prev = nullptr;
        block->next = quickBins[bin];
        quickBins[bin] = block;
        block->isFree = true;
        return;
    }
    
    
    block->isFree = true;

    if (block->next && block->next->isFree) {
        block->size += sizeof(BlockHeader) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    if (block->prev && block->prev->isFree)  {
        block->prev->size += sizeof(BlockHeader) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}


/**
 * Get a new VM Page from Linux Kernel
 */
void* HeapManager::GetNewVMPageFromKernel(int units)
{
    void* vmPage = mmap(
        0,
        4096,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE,
        0,
        0
    );

    if (vmPage == MAP_FAILED) {
        std::cerr << "[ERROR] VM Page Allocation Failed" << std::endl;
        return nullptr;
    }

    return vmPage;
}

/**
 * Return all vm pages to the kernel
 */
void HeapManager::CleanUp()
{
    Arena* current = pages;
    while (current != nullptr) {
        Arena* next = current->nextArena;

        if (munmap(current, ARENA_SIZE) == -1) {
            std::cerr << "[ERROR] munmap failed for " << current << std::endl;
        }

        current = next;
    }

    pages = nullptr;
}

/**
 * Dump the heap
 */
void HeapManager::heapDump()
{
    Arena* currArena = pages;
    int arenaCount = 0;

    while (currArena) {
        std::cout << "Arena [" << arenaCount << "] @ " << currArena << "\n";
        BlockHeader* currBlock = currArena->head;

        while (currBlock) {
            std::cout << "  Block @ " << currBlock
                      << " | Size: " << currBlock->size
                      << " | Free: " << ((currBlock->isFree) ? "YES" : "NO") << "\n";
            currBlock = currBlock->next;
        }
        currArena = currArena->nextArena;
    }
}

