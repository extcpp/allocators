#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE allocator_wrapper
#include <boost/test/unit_test.hpp>

#define private public
#include <ext/allocators/stl_wrapper.hpp>
#undef private

#include <ext/allocators/bitmap.hpp>
#include <ext/allocators/blob.hpp>

using namespace alloc;

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

BOOST_AUTO_TEST_CASE(test_functions) {
    test::dummy_allocator a;
    alloc::allocator_wrapper<int, test::dummy_allocator> w{&a};

    {
        alloc::allocator_wrapper<int, test::dummy_allocator> other = w;
        BOOST_CHECK_EQUAL(other._allocator, &a);
    }

    {
        alloc::allocator_wrapper<double, test::dummy_allocator> other = w;
        BOOST_CHECK_EQUAL(other._allocator, &a);
    }

    {
        alloc::allocator_wrapper<int, test::dummy_allocator> other(nullptr);
        BOOST_CHECK_EQUAL(other == w, false);

        other = w;
        BOOST_CHECK_EQUAL(other._allocator, &a);
        BOOST_CHECK_EQUAL(other == w, true);
    }

    {
        alloc::allocator_wrapper<double, test::dummy_allocator> other(nullptr);
        BOOST_CHECK_EQUAL(other == w, false);

        other = w;
        BOOST_CHECK_EQUAL(other._allocator, &a);
        BOOST_CHECK_EQUAL(other == w, true);
    }

    {
        auto* ptr = w.allocate(1);
        char* cptr = reinterpret_cast<char*>(ptr);
        BOOST_CHECK_EQUAL(cptr, a.data + sizeof(std::size_t));
        BOOST_CHECK_EQUAL(a.allocated, true);

        w.deallocate(ptr, 1);
        BOOST_CHECK_EQUAL(a.allocated, false);
    }

    { BOOST_CHECK_THROW(w.allocate(65), std::bad_alloc); }
}

BOOST_AUTO_TEST_CASE(test_std_functionality) {
    alloc::blob_allocator<64, 8> a;
    alloc::allocator_wrapper<int, alloc::blob_allocator<64, 8>> w(&a);

    auto* ptr = w.allocate(1);

    auto* block_ptr = reinterpret_cast<std::byte*>(ptr) - std::max(sizeof(size_t), alignof(int));
    BOOST_CHECK_GE(reinterpret_cast<std::byte*>(block_ptr), reinterpret_cast<char*>(&a));

    auto block_size = *reinterpret_cast<std::size_t*>(block_ptr);
    BOOST_CHECK_GE(block_size, sizeof(int));

    memory_block block{block_ptr, block_size};
    BOOST_CHECK(a.owns(block));

    w.deallocate(ptr, 1);
}

BOOST_AUTO_TEST_CASE(test_vector_allocator) {
    using A = alloc::bitmap_allocator<alloc::blob_allocator<128, 8>, 4, 32, 4>;
    using W = alloc::allocator_wrapper<int, A>;

    A a;
    W w(&a);

    {
        std::vector<int, W> vec(w);

        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        vec.push_back(4);
        vec.push_back(5);

        BOOST_CHECK_LE(reinterpret_cast<char*>(&a), reinterpret_cast<char*>(&vec[0]));
        BOOST_CHECK_GE(reinterpret_cast<char*>(&a) + sizeof(a), reinterpret_cast<char*>(&vec[0]));
    }
}
