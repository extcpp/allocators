#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE blob_allocator
#include <boost/test/unit_test.hpp>

#define private public
#include "blob_allocator.hpp"
#undef private

BOOST_AUTO_TEST_CASE(test_allocate) {
    alloc::blob_allocator<64, 8> a;

    auto block = a.allocate(64, 8);
    BOOST_CHECK_EQUAL(block.size, 64);
    BOOST_CHECK_EQUAL(reinterpret_cast<intptr_t>(block.ptr) % 8, 0);
    BOOST_CHECK_EQUAL(a._allocated, true);

    auto block2 = a.allocate(64, 8);
    BOOST_CHECK(block2.ptr == nullptr);
    BOOST_CHECK_EQUAL(block2.size, 0);
    BOOST_CHECK_EQUAL(a._allocated, true);

    a.deallocate(block);
    BOOST_CHECK_EQUAL(a._allocated, false);

    block = a.allocate(64, 8);
    BOOST_CHECK_EQUAL(block.size, 64);
    BOOST_CHECK_EQUAL(reinterpret_cast<intptr_t>(block.ptr) % 8, 0);
    BOOST_CHECK_EQUAL(a._allocated, true);

    a.deallocate(block);
    BOOST_CHECK_EQUAL(a._allocated, false);
}
