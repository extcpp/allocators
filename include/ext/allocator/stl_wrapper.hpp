#ifndef INCLGUARD_allocator_wrapper_hpp
#define INCLGUARD_allocator_wrapper_hpp

#include "detail_block.hpp"
#include <boost/align/align_up.hpp>

#include <algorithm>
#include <memory>
#include <type_traits>

namespace alloc {
template<typename T, typename Allocator, std::size_t Alignment = alignof(T)>
class allocator_wrapper {
    public:
    template<typename U>
    struct rebind {
        using other = allocator_wrapper<U, Allocator, Alignment>;
    };

    public:
    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    static constexpr std::size_t alignment = Alignment;

    allocator_wrapper(Allocator* allocator) noexcept : _allocator(allocator) {}

    template<typename U>
    allocator_wrapper(allocator_wrapper<U, Allocator, Alignment> const& other) noexcept
        : _allocator{other._allocator} {}

    template<typename U>
    allocator_wrapper& operator=(allocator_wrapper<U, Allocator, Alignment> const& other) noexcept {
        _allocator = other._allocator;
        return *this;
    }

    T* allocate(std::size_t n) {
        constexpr std::size_t actual_allignment = std::max(sizeof(size_t), alignment);
        memory_block block = _allocator->allocate(actual_allignment + sizeof(T) * n, actual_allignment);
        if (block.data) {
            *reinterpret_cast<size_t*>(block.data) = block.size;
            return reinterpret_cast<T*>(reinterpret_cast<char*>(block.data) + actual_allignment);
        } else
            throw std::bad_alloc{};
    }

    void deallocate(T* ptr, std::size_t) {
        constexpr std::size_t actual_allignment = std::max(sizeof(size_t), alignment);

        memory_block block{reinterpret_cast<std::byte*>(ptr) - actual_allignment,
                           *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(ptr) - actual_allignment)};

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

#if 0
template<typename Allocator, int Options = 0>
struct deleter {
    using allocator_t = Allocator;

    template<typename T>
    void operator()(T* ptr) const {
        static_assert(Allocator::actual_size(sizeof(T), alignof(T)) == sizeof(T),
                      "allocator must be able to allocate exactly the size of given type");

        if (ptr) {
            ptr->~T();
            allocator_t::instance().deallocate(memory_block.data, sizeof(T));
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
            //            allocator->deallocate(memory_block.data, sizeof(Type));
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
            //            allocator_t::instance().deallocate(memory_block.data, size);
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
            //           allocator->deallocate(memory_block.data, size);
        }
    }

    allocator_t* allocator;
    std::size_t size;
};


/// returns a unique_ptr holding the given type created with a global instance of Allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(std::nullptr_t, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(sizeof(Type), alignof(Type)) == sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator>>> {
    auto block = Allocator::instance().allocate(sizeof(Type), alignof(Type));
    return std::unique_ptr<Type, deleter<Allocator>>{new (block.data) Type{std::forward<Args>(args)...}};
}

/// returns a unique_ptr holding the given type created with a global instance of Allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(std::nullptr_t, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(sizeof(Type), alignof(Type)) != sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator, deleter_options::divergent_size>>> {
    auto block = Allocator::instance().allocate(sizeof(Type), alignof(Type));
    return {new (block.data) Type{std::forward<Args>(args)...}, {block.size}};
}

/// returns a unique_ptr holding the given type created with the given allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(Allocator* a, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(sizeof(Type), alignof(Type)) == sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator, deleter_options::local>>> {
    auto block = a->allocate(sizeof(Type), alignof(Type));
    return {new (block.data) Type{std::forward<Args>(args)...}, {a}};
}

/// returns a unique_ptr holding the given type created with the given allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(Allocator* a, Args&&... args) -> std::enable_if_t<
    Allocator::actual_size(sizeof(Type), alignof(Type)) != sizeof(Type),
    std::unique_ptr<Type, deleter<Allocator, deleter_options::divergent_size | deleter_options::local>>> {
    auto block = a->allocate(sizeof(Type), alignof(Type));
    return {new (block.data) Type{std::forward<Args>(args)...}, {a, block.size}};
}

#endif

} // namespace alloc

#endif
