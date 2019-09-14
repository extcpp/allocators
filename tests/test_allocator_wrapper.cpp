#include <gtest/gtest.h>

#define private public
#include <ext/allocators/stl_wrapper.hpp>
#undef private

#include <ext/allocators/bitmap.hpp>
#include <ext/allocators/blob.hpp>

using namespace EXT_ALLOCATOR_NAMESPACE;

namespace test {
struct dummy_allocator {
    dummy_allocator() : allocated{false} {}

    memory_block allocate(std::size_t /*aligenment*/, std::size_t size) {
        if (size <= 64 && allocated == false) {
            allocated = true;
            return {data, size};
        } else
            return {nullptr, 0};
    }

    void deallocate(memory_block block) {
        if (owns(block))
            allocated = false;
        else
            throw std::runtime_error{"bad block"};
    }

    bool owns(memory_block block) {
        return block.data == data && block.size <= 64;
    }

    std::byte data[64];
    bool allocated;
};
} // namespace test

TEST(wrapper, test_functions) {
    test::dummy_allocator a;
    EXT_ALLOCATOR_NAMESPACE::allocator_wrapper<int, test::dummy_allocator, std::max(alignof(int), alignof(double))> w{&a};

    {
        EXT_ALLOCATOR_NAMESPACE::allocator_wrapper<int, test::dummy_allocator, 8> other = decltype(w)::rebind<int>::other(w);
        EXPECT_EQ(other._allocator, &a);
    }

    {
        EXT_ALLOCATOR_NAMESPACE::allocator_wrapper<double, test::dummy_allocator> other =
            decltype(w)::rebind<double>::other(w);
        EXPECT_EQ(other._allocator, &a);
    }

    {
        EXT_ALLOCATOR_NAMESPACE::allocator_wrapper<int, test::dummy_allocator, 8> other(nullptr);
        //        EXPECT_EQ(other == w, false);

        other = w;
        EXPECT_EQ(other._allocator, &a);
        //       EXPECT_EQ(other == w, true);
    }

    {
        EXT_ALLOCATOR_NAMESPACE::allocator_wrapper<double, test::dummy_allocator> other(nullptr);
        //       EXPECT_EQ(other == w, false);

        other = w;
        EXPECT_EQ(other._allocator, &a);
        //        EXPECT_EQ(other == w, true);
    }

    {
        auto* ptr = w.allocate(1);
        auto* bptr = reinterpret_cast<std::byte*>(ptr);
        EXPECT_EQ(bptr, a.data + sizeof(std::size_t));
        EXPECT_EQ(a.allocated, true);

        w.deallocate(ptr, 1);
        EXPECT_EQ(a.allocated, false);
    }

    { EXPECT_THROW(w.allocate(65), std::bad_alloc); }
}

TEST(wrapper, test_std_functionality) {
    EXT_ALLOCATOR_NAMESPACE::blob_allocator<8, 64> a;
    EXT_ALLOCATOR_NAMESPACE::allocator_wrapper<int, EXT_ALLOCATOR_NAMESPACE::blob_allocator<8, 64>> w(&a);

    auto* ptr = w.allocate(1);

    auto* block_ptr = reinterpret_cast<std::byte*>(ptr) - std::max(sizeof(size_t), alignof(int));
    EXPECT_GE(reinterpret_cast<std::byte*>(block_ptr), reinterpret_cast<std::byte*>(&a));

    auto block_size = *reinterpret_cast<std::size_t*>(block_ptr);
    EXPECT_GE(block_size, sizeof(int));

    memory_block block{block_ptr, block_size};
    EXPECT_TRUE(a.owns(block));

    w.deallocate(ptr, 1);
}

TEST(wrapper, test_vector_allocator) {
    using A = EXT_ALLOCATOR_NAMESPACE::bitmap_allocator<EXT_ALLOCATOR_NAMESPACE::blob_allocator<8, 128>, 4, 32, 4>;
    using W = EXT_ALLOCATOR_NAMESPACE::allocator_wrapper<int, A>;

    A a;
    W w(&a);

    {
        std::vector<int, W> vec(w);

        try {
            vec.push_back(1);
            vec.push_back(2);
            vec.push_back(3);
            vec.push_back(4);
            vec.push_back(5);
        } catch (std::exception const& ex) {
            std::cerr << "error during push_back: " << ex.what();
            throw ex;
        }

        EXPECT_LE(reinterpret_cast<char*>(&a), reinterpret_cast<char*>(&vec[0])) << "dfds";
        EXPECT_GE(reinterpret_cast<char*>(&a) + sizeof(a), reinterpret_cast<char*>(&vec[0])) << "dfs";
    }
}
