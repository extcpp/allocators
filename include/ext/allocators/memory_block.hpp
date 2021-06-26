#ifndef EXT_ALLOCATORS_DETAIL_BLOCK_HEADER
#define EXT_ALLOCATORS_DETAIL_BLOCK_HEADER
#include "config.hpp"
#include <cassert>
#include <cstddef>
#include <stdexcept>

#ifdef EXTALLOC_EXCEPTIONS
#    define EXTALLOC_NOEXCEPT noexcept(false)
#else
#    define EXTALLOC_NOEXCEPT noexcept
#endif // EXTALLOC_EXCEPTIONS

namespace EXT_ALLOCATOR_NAMESPACE {

template<typename T>
std::enable_if_t<std::is_pointer_v<T>, T>
inline constexpr align_up(T addr, std::size_t alignment) {
    return reinterpret_cast<T>(((reinterpret_cast<std::uintptr_t>(addr) + (alignment - 1)) / alignment) * alignment);
}

struct memory_block;
inline bool owns_block(std::byte const* data, std::size_t size, memory_block const& block) EXTALLOC_NOEXCEPT;

struct memory_block {
    memory_block() noexcept = default;
    memory_block(std::byte* data_, std::size_t size_) : data{data_}, size{size_} {}
    memory_block(std::nullptr_t) : data(nullptr), size{0} {}

    explicit operator bool() {
        return data != nullptr;
    }

    bool owns(memory_block const& other) const EXTALLOC_NOEXCEPT {
        return owns_block(data, size, other);
    };

    std::byte* data;    // pointer to data
    std::size_t size;   // byte size

};

inline bool owns_block(std::byte const* data, std::size_t size, memory_block const& block) EXTALLOC_NOEXCEPT {

    // freeing an nullptr is no problem. We should not fail in `assert(owns(block))`
    if (block.data == nullptr) {
        assert(block.size == 0);
        return true;
    }

    if (block.data >= data) {
        if (block.data + block.size <= data + size) {
            return true;
        } else {
#ifdef EXTALLOC_EXCEPTIONS
            throw std::runtime_error("allocation begins in this block but is not contained within it");
#endif // EXTALLOC_EXCEPTIONS
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

#endif // EXT_ALLOCATORS_DETAIL_BLOCK_HEADER
