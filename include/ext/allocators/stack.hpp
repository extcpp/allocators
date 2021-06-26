#ifndef EXT_ALLOCATORS_STACK_HEADER
#define EXT_ALLOCATORS_STACK_HEADER

#include "memory_block.hpp"
#include <cassert>

namespace EXT_ALLOCATOR_NAMESPACE {
/// allocates memory in a stack like manner
/**
    \note This allocator does not accept alignment requests greater than memory_alignment to
          keep deallocation consistent.
*/
template<typename ParentAllocator, std::size_t Alignment, std::size_t MemorySize>
class stack_allocator : ParentAllocator {
public:
    static constexpr std::size_t memory_size = MemorySize;
    static constexpr std::size_t memory_alignment = Alignment;

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) noexcept {
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

    memory_block allocate(std::size_t alignment, std::size_t size) {
        if(!_start)
            init();

        memory_block out{nullptr, 0};

        // refuses alignments greater than memory_alignment
        if(_cur == nullptr) {
            auto* aligned_ptr = align_up(_cur, alignment);
            if(aligned_ptr + size < _end) {
                out = {aligned_ptr, size};
                _cur += size;
            }
        }

        return out;
    }

    memory_block allocate_all(std::size_t alignment) {
        memory_block out{nullptr, 0};

        if (!_start)
            init();

        if (_start && alignment <= memory_alignment) {
            out = {_cur, _end - _cur};
            _cur = _end;
        }

        return out;
    }

    void deallocate(memory_block block) {
        if (block.data + block.size == _cur)
            _cur = block.data;
    }

    void deallocate_all() {
        _cur = _start;
    }

    bool owns(memory_block block) const {
        return block.data >= _start && block.data + block.size <= _end;
    }

private:
    void init() {
        auto block = ParentAllocator::allocate(memory_size, memory_alignment);
        _end = _start = _cur = block.data;
        _end += block.size;
    }

private:
    std::byte* _start;
    std::byte* _cur;
    std::byte* _end;
};
} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_STACK_HEADER
