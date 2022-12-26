/**
 * @brief PyTorch CUDA caching allocator simulator
 * @date 12/19/2022
*/
#ifndef ALLOCATOR_SIM_H
#define ALLOCATOR_SIM_H

#include <iostream>

#include "allocator_utils.h"
#include "allocator_conf.h"

class allocatorSim {
private:
    BlockPool small_blocks;
    BlockPool large_blocks;
    allocatorConfig allocator_config;
    size_t memory_address = 1;

private:
    size_t round_size(size_t ori_size);

    BlockPool& get_pool(size_t size, int stream);
    
    size_t get_allocation_size(size_t size);

    bool get_free_block(AllocParams& p);

    bool trigger_free_memory_callbacks(AllocParams& p);

    void garbage_collect_cached_blocks();

    bool alloc_block(AllocParams& p, bool isRetry);

    bool release_available_cached_blocks(AllocParams& p);

    bool release_cached_blocks();

    bool should_split(const Block* block, size_t size);

    void free_block(Block* block);

    size_t try_merge_blocks(Block* dst, Block* src, BlockPool& pool);

public:
    allocatorSim();

    ~allocatorSim();

    void test_allocator();

    Block* malloc(int device, size_t orig_size, int stream);

    void free(Block* block);
};

#endif  // ALLOCATOR_SIM_H