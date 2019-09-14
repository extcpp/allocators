#ifndef INCLGUARD_allocator_wrapper_hpp
#define INCLGUARD_allocator_wrapper_hpp

#include "detail_block.hpp"
#include <boost/align/align_up.hpp>

#include <algorithm>
#include <memory>
#include <type_traits>

namespace EXT_ALLOCATOR_NAMESPACE {
template<typename T, typename Allocator, std::size_t Alignment = alignof(T)>
class allocator_wrapper {
    public:
    static constexpr std::size_t alignment = Alignment;

    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    template<typename U>
    struct rebind {
        using other = allocator_wrapper<U, Allocator, Alignment>;
    };

    allocator_wrapper(Allocator* allocator) noexcept : _allocator(allocator) {}

    template<typename U>
    allocator_wrapper(allocator_wrapper<U, Allocator, Alignment> const& other) noexcept
        : _allocator{other._allocator} {}

    template<typename U>
    allocator_wrapper& operator=(allocator_wrapper<U, Allocator, Alignment> const& other) noexcept {
        _allocator = other._allocator;
        return *this;
    }

    value_type* allocate(std::size_t n) const {
        // const is require by the standard and it is ok as we work on a pointer (_allocator)
        constexpr std::size_t actual_allignment = std::max(sizeof(size_t), alignment);

        memory_block block = _allocator->allocate(actual_allignment,
                                                  actual_allignment /*storage for size*/ +
                                                      sizeof(value_type) * n /*storage for real data*/);

        if (block.data) {
            *reinterpret_cast<size_t*>(block.data) = block.size; // save block size before type
            return reinterpret_cast<T*>(reinterpret_cast<char*>(block.data) + actual_allignment);
        } else {
            throw std::bad_alloc{};
        }
    }

    void deallocate(T* ptr, std::size_t) const {
        constexpr std::size_t actual_allignment = std::max(sizeof(size_t), alignment);

        // restore original block
        std::byte* data = reinterpret_cast<std::byte*>(ptr) - actual_allignment;
        std::size_t size = *reinterpret_cast<size_t*>(data);
        memory_block block{data, size};

        assert(_allocator->owns(block));
        _allocator->deallocate(block);
    }

    template<typename T_, typename U, typename Allocator_>
    friend bool operator==(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs);

    template<typename T_, typename U, typename Allocator_>
    friend bool operator!=(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs);

    private:
    Allocator* _allocator;
};

template<typename T_, typename U, typename Allocator_>
bool operator==(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs) {
    return lhs._allocator == rhs._allocator;
}

template<typename T_, typename U, typename Allocator_>
bool operator!=(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs) {
    return !(lhs == rhs);
}


struct deleter_options {
    enum { divergent_size = 0b01, local = 0b10 };
};

#if 1
template<typename Allocator, int Options = 0>
struct deleter {
    using allocator_t = Allocator;

    template<typename T>
    void operator()(T* ptr) const {
        static_assert(Allocator::actual_size(sizeof(T), alignof(T)) == sizeof(T),
                      "allocator must be able to allocate exactly the size of given type");

        if (ptr) {
            ptr->~T();
            allocator_t::instance().deallocate(memory_block{reinterpret_cast<std::byte*>(ptr), sizeof(T)});
        }
    }
};

template<typename Allocator>
struct deleter<Allocator, deleter_options::local> {
    using allocator_t = Allocator;

    template<typename Type>
    void operator()(Type* ptr) const {
        static_assert(Allocator::actual_size(sizeof(Type), alignof(Type)) == sizeof(Type),
                      "allocator must be able to allocate exactly the size of given type");

        if (ptr) {
            ptr->~Type();
            allocator->deallocate(memory_block{reinterpret_cast<std::byte*>(ptr), sizeof(Type)});
        }
    }

    allocator_t* allocator;
};

template<typename Allocator>
struct deleter<Allocator, deleter_options::divergent_size> {
    using allocator_t = Allocator;

    template<typename Type>
    void operator()(Type* ptr) const {
        if (ptr) {
            ptr->~Type();
            allocator_t::instance().deallocate(memory_block{reinterpret_cast<std::byte*>(ptr), size});
        }
    }

    std::size_t size;
};

template<typename Allocator>
struct deleter<Allocator, deleter_options::divergent_size | deleter_options::local> {
    using allocator_t = Allocator;

    template<typename Type>
    void operator()(Type* ptr) const {
        if (ptr) {
            ptr->~Type();
            allocator->deallocate(memory_block{reinterpret_cast<std::byte*>(ptr), size});
        }
    }

    allocator_t* allocator;
    std::size_t size;
};


/// returns a unique_ptr holding the given type created with a global instance of Allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(std::nullptr_t, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(alignof(Type), sizeof(Type)) == sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator>>> {
    auto block = Allocator::instance().allocate(alignof(Type), sizeof(Type));
    return std::unique_ptr<Type, deleter<Allocator>>{new (block.data) Type{std::forward<Args>(args)...}};
}

/// returns a unique_ptr holding the given type created with a global instance of Allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(std::nullptr_t, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(alignof(Type), sizeof(Type)) != sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator, deleter_options::divergent_size>>> {
    auto block = Allocator::instance().allocate(alignof(Type), sizeof(Type));
    return {new (block.data) Type{std::forward<Args>(args)...}, {block.size}};
}

/// returns a unique_ptr holding the given type created with the given allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(Allocator* a, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(alignof(Type), sizeof(Type)) == sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator, deleter_options::local>>> {
    auto block = a->allocate(alignof(Type), sizeof(Type));
    return {new (block.data) Type{std::forward<Args>(args)...}, {a}};
}

/// returns a unique_ptr holding the given type created with the given allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(Allocator* a, Args&&... args) -> std::enable_if_t<
    Allocator::actual_size(alignof(Type), sizeof(Type)) != sizeof(Type),
    std::unique_ptr<Type, deleter<Allocator, deleter_options::divergent_size | deleter_options::local>>> {
    auto block = a->allocate(alignof(Type), sizeof(Type));
    return {new (block.data) Type{std::forward<Args>(args)...}, {a, block.size}};
}

#endif

} // namespace EXT_ALLOCATOR_NAMESPACE

#endif
