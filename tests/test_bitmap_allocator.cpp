#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE bitmap_allocator
#include <boost/test/unit_test.hpp>

#include <ext/allocators/blob.hpp>
#define private public //geil!!! ballern!!
#include <ext/allocators/bitmap.hpp>
#undef private

BOOST_AUTO_TEST_CASE(test_allocate) {
    alloc::bitmap_allocator<alloc::blob_allocator<64, 8>, 4, 16, 8> a;
    BOOST_CHECK(a._block.data == nullptr);
    BOOST_CHECK_EQUAL(a._block.size, 0);

    auto block = a.allocate(32, 8);
    BOOST_CHECK_EQUAL(block.size, 32);
    BOOST_CHECK_EQUAL(reinterpret_cast<intptr_t>(block.data) % 8, 0);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0xff00);

    auto block2 = a.allocate(32, 8);
    BOOST_CHECK_EQUAL(block2.size, 32);
    BOOST_CHECK(block2.data != block.data);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0x0);

    a.deallocate(block);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0xff);

    a.deallocate(block2);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0xffff);
}

BOOST_AUTO_TEST_CASE(test_split_pattern) {
    alloc::bitmap_allocator<alloc::blob_allocator<120, 1>, 1, 120, 1> a;
    auto block1 = a.allocate(62, 1);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0xc000000000000000);
    BOOST_CHECK_EQUAL(a._free_blocks[1], 0x00ffffffffffffff);

    auto block2 = a.allocate(4, 1);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0x0000000000000000);
    BOOST_CHECK_EQUAL(a._free_blocks[1], 0x00fffffffffffffc);

    a.deallocate(block1);
    a.deallocate(block2);
}

BOOST_AUTO_TEST_CASE(test_rejected_sizes) {
    alloc::bitmap_allocator<alloc::blob_allocator<256, 1>, 1, 256, 1> a;
    auto block = a.allocate(129, 1);
    BOOST_CHECK(block.data == nullptr);
    BOOST_CHECK_EQUAL(block.size, 0);

    block = a.allocate(512, 1);
    BOOST_CHECK(block.data == nullptr);
    BOOST_CHECK_EQUAL(block.size, 0);
}

BOOST_AUTO_TEST_CASE(test_chunk_alignment) {
    alloc::bitmap_allocator<alloc::blob_allocator<128, 8>, 1, 128, 8> a;
    auto block1 = a.allocate(4, 1);
    BOOST_CHECK(block1.data != nullptr);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0xfffffffffffffff0);
    BOOST_CHECK_EQUAL(a._free_blocks[1], 0xffffffffffffffff);

    auto block2 = a.allocate(8, 8);
    BOOST_CHECK(block2.data != nullptr);
    BOOST_CHECK_EQUAL(a._free_blocks[0], 0xffffffffffff00f0);
    BOOST_CHECK_EQUAL(a._free_blocks[1], 0xffffffffffffffff);

    a.deallocate(block1);
    a.deallocate(block2);
}
