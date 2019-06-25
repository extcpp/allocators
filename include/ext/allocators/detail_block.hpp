#pragma once
#ifndef INCLGUARD_memory_block_hpp
#    define INCLGUARD_memory_block_hpp
#    include <cstddef>

namespace alloc {
struct memory_block {
    memory_block() noexcept = default;
    memory_block(std::byte* data_, std::size_t size_) : data{data_}, size{size_} {}
    memory_block(std::nullptr_t) : data(nullptr), size{0} {}

    explicit operator bool() {
        return data != nullptr;
    }

    std::byte* data;
    std::size_t size;
};

inline bool operator==(memory_block lhs, memory_block rhs) {
    return lhs.data == rhs.data && lhs.size == rhs.size;
}

inline bool operator!=(memory_block lhs, memory_block rhs) {
    return !(lhs == rhs);
}

} // namespace alloc

#endif
