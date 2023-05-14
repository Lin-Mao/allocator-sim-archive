/**
 * @brief PyTorch CUDA caching allocator
 * @date 12/19/2022
*/

#include <array>
#include <cassert>

#include "allocator_simulator.h"

namespace c10 {
namespace cuda {
namespace AllocatorSim {


static bool BlockComparator(const Block* a, const Block* b) {
    if (a->stream != b->stream) {
        return (int)a->stream < (int)b->stream;
    }
    if (a->size != b->size) {
        return a->size < b->size;
    }
    return (uint64_t)a->ptr < (uint64_t)b->ptr;
}

allocatorSim::allocatorSim()
    : max_reserved_bytes(0),
    current_reserved_bytes(0),
    max_allocated_bytes(0),
    current_allocated_bytes(0) {
    small_blocks = BlockPool(BlockComparator, true);
    large_blocks = BlockPool(BlockComparator, false);

    allocator_prof = new allocatorProf();

#ifdef DUMP_INFO_TO_FILE_DEBUGGING
    std::ofstream out1(get_dump_file_path() + "allocator.txt");
    out1 << "";
    out1.close();
    std::ofstream out2(get_dump_file_path() + "simulator.txt");
    out2 << "";
    out2.close();
    std::ofstream out3(get_dump_file_path() + "allocator_pools.txt");
    out3 << "";
    out3.close();
    std::ofstream out4(get_dump_file_path() + "simulator_pools.txt");
    out4 << "";
    out4.close();
    std::ofstream out5(get_dump_file_path() + "allocator_seg_events.txt");
    out5 << "";
    out5.close();
    std::ofstream out6(get_dump_file_path() + "simulator_seg_events.txt");
    out6 << "";
    out6.close();
    std::ofstream out7(get_dump_file_path() + "allocator_mem_layout.txt");
    out7 << "";
    out7.close();
    std::ofstream out8(get_dump_file_path() + "simulator_mem_layout.txt");
    out8 << "";
    out8.close();
#endif

}

allocatorSim::~allocatorSim() {
    // std::cout << "Max allocated size: " << max_allocated_bytes << " B ("
    //           << format_size(max_allocated_bytes) << ")" << std::endl;
    // std::cout << "Max reserved size: " << max_reserved_bytes << " B ("
    //           << format_size(max_reserved_bytes) << ")" << std::endl;

    delete allocator_prof;
}

void allocatorSim::test_allocator() {
    std::cout << "Hello allocator!" << std::endl;
}

size_t allocatorSim::round_size(size_t size) {
    auto min_block_size = allocatorConf::get_kMinBlockSize();
    if (size < min_block_size) {
        return min_block_size;
    } else if (size > allocatorConf::get_roundup_bypass_threshold()) {
        return min_block_size * ((size + min_block_size - 1) / min_block_size);
    } else {
        auto divisions = allocatorConf::get_roundup_power2_divisions();
        if (divisions > 0 && size > (min_block_size * divisions)) {
        // return roundup_power2_next_division(size, divisions);
        // not taken
        return size;
        } else {
        return min_block_size * ((size + min_block_size - 1) / min_block_size);
        }
    }
}

BlockPool& allocatorSim::get_pool(size_t size, int stream) {
    if (size <= allocatorConf::get_kSmallSize()) {
        return small_blocks;
    } else {
        return large_blocks;
    }
}

size_t allocatorSim::get_grouped_allocation_size_sim(size_t size) {
    if (size < allocatorConf::_GROUPS[0]) {
        if (allocatorConf::_GROUPS[0] != std::numeric_limits<size_t>::max()) {
            return allocatorConf::_GROUPS[0];
        } else {
            auto tunablekRoundLarge = AllocatorSim::allocatorConf::get_kRoundLarge();
            return tunablekRoundLarge * ((size + tunablekRoundLarge - 1) / tunablekRoundLarge);
        }
    } else if (size < allocatorConf::_GROUPS[1]) {
        if (allocatorConf::_GROUPS[1] != std::numeric_limits<size_t>::max()) {
            return allocatorConf::_GROUPS[1];
        } else {
            auto tunablekRoundLarge = AllocatorSim::allocatorConf::get_kRoundLarge();
            return tunablekRoundLarge * ((size + tunablekRoundLarge - 1) / tunablekRoundLarge);
        }
    } else if (size < allocatorConf::_GROUPS[2]) {
        if (allocatorConf::_GROUPS[2] != std::numeric_limits<size_t>::max()) {
            return allocatorConf::_GROUPS[2];
        } else {
            auto tunablekRoundLarge = AllocatorSim::allocatorConf::get_kRoundLarge();
            return tunablekRoundLarge * ((size + tunablekRoundLarge - 1) / tunablekRoundLarge);
        }
    } else if (size < allocatorConf::_GROUPS[3]) {
        if (allocatorConf::_GROUPS[3] != std::numeric_limits<size_t>::max()) {
            return allocatorConf::_GROUPS[3];
        } else {
            auto tunablekRoundLarge = AllocatorSim::allocatorConf::get_kRoundLarge();
            return tunablekRoundLarge * ((size + tunablekRoundLarge - 1) / tunablekRoundLarge);
        }
    } else if (size < allocatorConf::_GROUPS[4]) {
        if (allocatorConf::_GROUPS[4] != std::numeric_limits<size_t>::max()) {
            return allocatorConf::_GROUPS[4];
        } else {
            auto tunablekRoundLarge = AllocatorSim::allocatorConf::get_kRoundLarge();
            return tunablekRoundLarge * ((size + tunablekRoundLarge - 1) / tunablekRoundLarge);
        }
    } else {
        auto tunablekRoundLarge = AllocatorSim::allocatorConf::get_kRoundLarge();
        return tunablekRoundLarge * ((size + tunablekRoundLarge - 1) / tunablekRoundLarge);
    }
}

void allocatorSim::set_group_enable_flag_sim(bool flag) {
    group_enable_flag_sim = flag;
}

size_t allocatorSim::get_allocation_size(size_t size) {
    if (group_enable_flag_sim && size > allocatorConf::get_kLargeBuffer()) {
        return get_grouped_allocation_size_sim(size);
    }
    if (size <= allocatorConf::get_kSmallSize()) {
        return allocatorConf::get_kSmallBuffer();
    } else if (size < allocatorConf::get_kMinLargeAlloc()) {
        return allocatorConf::get_kLargeBuffer();
    } else {
        auto round_large = allocatorConf::get_kRoundLarge();
        return round_large * ((size + round_large - 1) / round_large);
    }
}

// retrieve the block for a given simulator ptr
// may not be used since don't plan to handle release not caused by empty cache
Block* allocatorSim::retrieve_released_block(uint64_t ptr) {
    auto it = releasable_blocks.find(ptr);
    if (it == releasable_blocks.end()) {
        return nullptr;
    } else {
        auto block = it->second;
        releasable_blocks.erase(it);
        return block;
    }
}

bool allocatorSim::get_free_block(AllocParams& p) {
    BlockPool& pool = *p.pool;
    auto it = pool.blocks.lower_bound(&p.search_key);
    if (it == pool.blocks.end() || (*it)->stream != p.stream())
        return false;
    if ((p.size() >= allocatorConf::get_max_split_size()) &&
        ((*it)->size >= p.size() + allocatorConf::get_kLargeBuffer()))
        return false;
    p.block = *it;
    (*it)->gc_count = 0; // Denote this block has been used
    pool.blocks.erase(it);
    if (releasable_blocks.find(p.block->ptr) != releasable_blocks.end()) {
        releasable_blocks.erase(p.block->ptr);
    }
    return true;
}

bool allocatorSim::trigger_free_memory_callbacks(AllocParams& p) {
    // @todo(Lin-Mao): update reference?
    return true;
}

void allocatorSim::garbage_collect_cached_blocks() {
    // skip
}

bool allocatorSim::alloc_block(AllocParams& p, bool isRetry, void* o_ptr) {
    size_t size = p.alloc_size;
    uint64_t ptr = 0;
    device_allocator.allocate(ptr, size);

    // p.block = new Block(p.device(), p.stream(), size, p.pool, reinterpret_cast<uint64_t>(o_ptr));
    p.block = new Block(p.device(), p.stream(), size, p.pool, ptr);

    current_reserved_bytes += size;
    max_reserved_bytes = std::max(current_reserved_bytes, max_reserved_bytes);

    return true;
}

void allocatorSim::release_block(Block* block) {
    allocator_prof->update_segment_release(block);
    current_reserved_bytes -= block->size;
    auto* pool = block->pool;
    pool->blocks.erase(block);
    device_allocator.free(block->ptr, block->size);

#ifdef DUMP_INFO_TO_FILE_DEBUGGING
    std::ofstream out1(get_dump_file_path() + "simulator.txt", std::ios::app);
    out1 << get_global_op_id() << ": release: true, size: " << block->size
        << ", allocated: " << current_allocated_bytes
        << ", reserved: " << current_reserved_bytes << std::endl;
    out1.close();

    _active_segments.erase(block->ptr);

    std::ofstream out2(get_dump_file_path() + "simulator_mem_layout.txt", std::ios::app);
    out2 << "op_id: " << get_global_op_id() << " free: " << block->ptr << " size: " << block->size << std::endl;
    for (auto s : _active_segments) {
        out2 << "[" << s.first << ", " << s.first + s.second.second << ") ";
    }
    out2 << std::endl;
    out2.close();

    std::ofstream out3(get_dump_file_path() + "simulator_seg_events.txt", std::ios::app);
    out3 << "op_id: " << get_global_op_id() << ", release " << block->size << " bytes" << std::endl;
    out3.close();
#endif

    delete block;
}

bool allocatorSim::release_available_cached_blocks(AllocParams& p) {
    // @todo(Lin-Mao): todo
    return false;
}

bool allocatorSim::release_cached_blocks() {
    release_blocks(large_blocks);
    release_blocks(small_blocks);

    return true;
}

void allocatorSim::empty_cache() {
    release_cached_blocks();
}

bool allocatorSim::should_split(const Block* block, size_t size) {
    size_t remaining = block->size - size;
    if (block->pool->is_small) {
        return remaining >= allocatorConf::get_kMinBlockSize();
    } else {
        return (size < allocatorConf::get_max_split_size()) &&
            (remaining > allocatorConf::get_kSmallSize());
    }
}

Block* allocatorSim::malloc(int device, size_t orig_size, int stream, void* o_ptr) {
    size_t size = round_size(orig_size);
    auto& pool = get_pool(size, stream);
    const size_t alloc_size = get_allocation_size(size);
    AllocParams params(device, size, stream, &pool, alloc_size);

#ifdef DUMP_INFO_TO_FILE_DEBUGGING
    std::ofstream out1(get_dump_file_path() + "simulator_pools.txt", std::ios::app);
    out1 << "op_id: " << get_global_op_id() << std::endl;
    for (auto b : small_blocks.blocks) {
        out1 << b->size << " ";
    }
    out1 << std::endl;
    for (auto b : large_blocks.blocks) {
        out1 << b->size << " ";
    }
    out1 << std::endl;
    out1.close();
#endif

    bool block_found = get_free_block(params)
        // Trigger callbacks and retry search
        || (trigger_free_memory_callbacks(params) && get_free_block(params));

    bool real_alloc = false;
    if (!block_found) {
        // Do garbage collection if the flag is set.
        if (UNLIKELY(allocatorConf::get_garbage_collection_threshold() > 0.0)) {
            garbage_collect_cached_blocks();
        }
        // Attempt allocate
        block_found = alloc_block(params, false, o_ptr)
            // Free enough available cached blocks to satisfy alloc and retry
            // alloc.
            || (release_available_cached_blocks(params) &&
                alloc_block(params, false, o_ptr))
            // Free all non-split cached blocks and retry alloc.
            || (release_cached_blocks() && alloc_block(params, true, o_ptr));

        real_alloc = block_found;
        if (block_found) {
            allocator_prof->update_segment_create(params.block, alloc_size);
        }
    }

    if (!block_found) {
        // OOM
        assert(block_found);
    }

    assert(params.block != nullptr && params.block->ptr != 0);

    Block* block = params.block;
    Block* remaining = nullptr;

    // const bool already_split = block->is_split();

    bool split_flag = false;
    auto before_split_size = block->size;
    if (should_split(block, size)) {
        split_flag = true;
        remaining = block;

        block = new Block(device, stream, size, &pool, block->ptr);
        block->prev = remaining->prev;
        if (block->prev) {
            block->prev->next = block;
        }
        block->next = remaining;

        remaining->prev = block;
        remaining->ptr = remaining->ptr + static_cast<uint64_t>(size);
        remaining->size -= size;

        // This won't be taken
        auto iter = pool.blocks.find(remaining);
        if (iter != pool.blocks.end()) {
            std::cout << "iter != pool.blocks.end()" << std::endl;
        }

        bool inserted = pool.blocks.insert(remaining).second;

        assert(inserted);
    }

    block->allocated = true;

    current_allocated_bytes += block->size;
    max_allocated_bytes = std::max(current_allocated_bytes, max_allocated_bytes);

    allocator_prof->update_block_allocate(block);

#ifdef DUMP_INFO_TO_FILE_DEBUGGING
    std::ofstream out2(get_dump_file_path() + "simulator.txt", std::ios::app);
    out2 << get_global_op_id() << ": alloc: " << std::boolalpha << real_alloc << ", split: " << split_flag
        << ", orig_size: " << orig_size << ", size: " << size << ", alloc_size: " << alloc_size
        << ", before_split_size: " << before_split_size
        << ", allocated: " << current_allocated_bytes << ", reserved: " << current_reserved_bytes << std::endl;
    out2.close();

    if (real_alloc) {
        _active_segments.emplace(block->ptr, std::make_pair(get_global_op_id(), alloc_size));

        std::ofstream out3(get_dump_file_path() + "simulator_mem_layout.txt", std::ios::app);
        out3 << "op_id: " << get_global_op_id() << " malloc: " << block->ptr << " size: " << alloc_size << std::endl;
        for (auto s : _active_segments) {
            out3 << "[" << s.first << ", " << s.first + s.second.second << ") ";
        }
        out3 << std::endl;
        out3.close();

        std::ofstream out4(get_dump_file_path() + "simulator_seg_events.txt", std::ios::app);
        out4 << "op_id: " << get_global_op_id() << ", allocated " << alloc_size << " bytes" << std::endl;
        out4.close();
    }
#endif

    return block;
}

size_t allocatorSim::try_merge_blocks(Block* dst, Block* src, BlockPool& pool) {
    if (!src || src->allocated) {
        return 0;
    }

    if (dst->prev == src) { // [src dst]
        dst->ptr = src->ptr;
        dst->prev = src->prev;
        if (dst->prev) {
            dst->prev->next = dst;
        }
    } else { // [dest src]
        dst->next = src->next;
        if (dst->next) {
            dst->next->prev = dst;
        }
    }
    const size_t subsumed_size = src->size;
    dst->size += subsumed_size;
    auto erased = pool.blocks.erase(src);
    assert(erased);
    delete src;

    return subsumed_size;
}

void allocatorSim::release_blocks(BlockPool& pool) {
    auto it = pool.blocks.begin();
    while (it != pool.blocks.end()) {
        Block* block = *it;
        ++it;
        if (!block->prev && !block->next) {
            release_block(block);
        }
    }
}

void allocatorSim::free_block(Block* block) {
    // size_t original_block_size = block->size;

    auto& pool = *block->pool;
    int64_t net_change_inactive_split_blocks = 0;
    int64_t net_change_inactive_split_size = 0;

    const std::array<Block*, 2> merge_candidates = {block->prev, block->next};

    for (Block* merge_candidate : merge_candidates) {
        const int64_t subsumed_size = try_merge_blocks(block, merge_candidate, pool);
        if (subsumed_size > 0) {
            net_change_inactive_split_blocks -= 1;
            net_change_inactive_split_size -= subsumed_size;
        }
    }

    if (block->prev == nullptr && block->next == nullptr) {
        releasable_blocks.emplace(block->ptr, block);
    }

    bool inserted = pool.blocks.insert(block).second;
    assert(inserted);
}

void allocatorSim::free(Block* block) {
#ifdef DUMP_INFO_TO_FILE_DEBUGGING
    std::ofstream out1(get_dump_file_path() + "simulator_pools.txt", std::ios::app);
    out1 << "op_id: " << get_global_op_id() << std::endl;
    for (auto b : small_blocks.blocks) {
        out1 << b->size << " ";
    }
    out1 << std::endl;
    for (auto b : large_blocks.blocks) {
        out1 << b->size << " ";
    }
    out1 << std::endl;
    out1.close();
#endif

    block->allocated = false;

    // auto orig_block_ptr = block->ptr;
    auto orig_block_size = block->size;

    free_block(block);

    current_allocated_bytes -= orig_block_size;

#ifdef DUMP_INFO_TO_FILE_DEBUGGING
    std::ofstream out(get_dump_file_path() + "simulator.txt", std::ios::app);
    out << get_global_op_id() << ": free: true, size: " << orig_block_size
        << ", allocated: " << current_allocated_bytes
        << ", reserved: " << current_reserved_bytes << std::endl;
    out.close();
#endif

    // block is used to decide segment, keep size.
    allocator_prof->update_block_free(block, orig_block_size);
}

std::pair<size_t, size_t> allocatorSim::get_max_memory_usage() {
    return std::make_pair(max_allocated_bytes, max_reserved_bytes);
}

size_t allocatorSim::get_max_reserved_bytes() {
    return max_reserved_bytes;
}

size_t allocatorSim::get_max_allocated_bytes() {
    return max_allocated_bytes;
}

void allocatorSim::set_max_memory_usage(size_t allocated_bytes, size_t reserved_bytes) {
    max_allocated_bytes = allocated_bytes;
    max_reserved_bytes = reserved_bytes;
}

}  // namespace c10
}  // namespace cuda
}  // namespace AllocatorSim
