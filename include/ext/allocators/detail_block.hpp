#pragma once
#ifndef INCLGUARD_memory_block_hpp
#    define INCLGUARD_memory_block_hpp
#    include <cstddef>
#    include <stdexcept>
#    include <cassert>
#    include "config.hpp"

#ifdef EXTALLOC_EXCEPTIONS
#define EXTALLOC_NOEXCEPT noexcept(false)
#else
#define EXTALLOC_NOEXCEPT noexcept
#endif

namespace EXT_ALLOCATOR_NAMESPACE {

struct memory_block;
inline bool owns_block(std::byte const* data, std::size_t size, memory_block const& block) EXTALLOC_NOEXCEPT;

struct memory_block {
    memory_block() noexcept = default;
    memory_block(std::byte* data_, std::size_t size_) : data{data_}, size{size_} {}
    memory_block(std::nullptr_t) : data(nullptr), size{0} {}

    explicit operator bool() {
        return data != nullptr;
    }

    std::byte* data;
    std::size_t size;

    bool owns(memory_block const& other) const EXTALLOC_NOEXCEPT {
        return owns_block(data, size, other);
    };
};

inline bool owns_block(std::byte const* data, std::size_t size, memory_block const& block) EXTALLOC_NOEXCEPT {

    // freeing an nullptr is no problem. We should not fail in `assert(owns(block))`
    if (block.data == nullptr) {
        assert(block.size == 0);
        return true;
    }

    if (block.data >= data) {
        if ( block.data + block.size <= data + size) {
            return true;
        } else  {
    #ifdef EXTALLOC_EXCEPTIONS
            throw std::runtime_error("allocation begins in this block but is not contained within it");
    #endif
            return false;
        }
    }
    return false;
}

inline bool operator==(memory_block lhs, memory_block rhs) {
    return lhs.data == rhs.data && lhs.size == rhs.size;
}

inline bool operator!=(memory_block lhs, memory_block rhs) {
    return !(lhs == rhs);
}

} // namespace EXT_ALLOCATOR_NAMESPACE

#endif
