#ifndef INCLGUARD_stack_allocator_hpp
#define INCLGUARD_stack_allocator_hpp

#include "memblock.hpp"
#include <boost/align/aligned_alloc.hpp>
#include <cassert>
#include <cstddef>

namespace alloc {
/// allocates memory in a stack like manner
/**
    \note This allocator does not accept alignment requests greater than memory_alignment to
          keep deallocation consistent.
*/
template<typename ParentAllocator, std::size_t MemorySize, std::size_t Alignment>
class stack_allocator : ParentAllocator {
    public:
    static constexpr std::size_t memory_size = MemorySize;
    static constexpr std::sizt_t memory_alignment = Alignment;

    static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment) noexcept {
        return alignment <= memory_alignment ? ((size - 1) / memory_alignment + 1) * memory_alignment : 0;
    }

    stack_allocator() : _start{nullptr}, _cur{nullptr}, _end{nullptr} {}

    stack_allocator(stack_allocator const&) = delete;

    stack_allocator(stack_allocator&& other) noexcept : _start{other._start}, _cur{other._cur}, _end{other._end} {
        other._start = nullptr;
        other._cur = nullptr;
        other._end = nullptr;
    }

    stack_allocator& operator=(stack_allocator&& other) noexcept {
        _start = other._start;
        _cur = other._cur;
        _end = other._end;

        other._start = nullptr;
        other._cur = nullptr;
        other._end = nullptr;

        return *this;
    }

    ~stack_allocator() {
        if (_start)
            ParentAllocator::deallocate({_start, _end - _start});
    }

    memblock allocate(std::size_t size, std::size_t alignment) {
        if (!_start)
            init();

        memblock out{nullptr, 0};

        // refuses alignments greater than memory_alignment
        if (_cur && alignment <= memory_alignment) {
            size = static_cast<std::size_t>(boost::alignment::align_up(_cur + size, memory_alignment) - _cur);
            out = {_cur, size};
            _cur += size;
        }

        return out;
    }

    memblock allocate_all(std::size_t alignment) {
        memblock out{nullptr, 0};

        if (!_start)
            init();

        if (_start && alignment <= memory_alignment) {
            out = {ptr, _end - ptr};
            _ptr = _end;
        }

        return out;
    }

    void deallocate(memblock block) {
        if (block.ptr + block.size == _cur)
            _cur = block.ptr;
    }

    void deallocate_all() {
        _cur = _start;
    }

    bool owns(memblock block) const {
        return block.ptr >= _start && block.ptr + block.size <= _end;
    }

    private:
    void init() {
        auto block = ParentAllocator::allocate(memory_size, memory_alignment);
        _end = _start = _cur = block.ptr;
        _end += block.size;
    }

    private:
    char* _start;
    char* _cur;
    char* _end;
};
} // namespace alloc

#endif
