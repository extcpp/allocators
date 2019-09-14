#include <gtest/gtest.h>

#include <ext/allocators/blob.hpp>

#define private public // geil!!! ballern!!
#include <ext/allocators/bitmap.hpp>
#undef private

TEST(bitmap, test_allocate) {
    constexpr std::size_t alignment = 8;
    constexpr std::size_t size = 64;
    constexpr std::size_t chunk_size = 4;
    constexpr std::size_t num_chunks = 16;

    EXT_ALLOCATOR_NAMESPACE::
        bitmap_allocator<EXT_ALLOCATOR_NAMESPACE::blob_allocator<alignment, size>, alignment, chunk_size, num_chunks>
            a;
    EXPECT_TRUE(a._block.data == nullptr);
    EXPECT_EQ(a._block.size, 0);

    auto block = a.allocate(alignment, 32);
    EXPECT_EQ(block.size, 32);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(block.data) % alignment, 0);
    EXPECT_EQ(a._free_blocks[0], 0xff00);

    auto block2 = a.allocate(alignment, 32);
    EXPECT_EQ(block2.size, 32);
    EXPECT_TRUE(block2.data != block.data);
    EXPECT_EQ(a._free_blocks[0], 0x0);

    a.deallocate(block);
    EXPECT_EQ(a._free_blocks[0], 0xff);

    a.deallocate(block2);
    EXPECT_EQ(a._free_blocks[0], 0xffff);
}

TEST(bitmap, test_split_pattern) {
    constexpr std::size_t alignment = 1;
    constexpr std::size_t size = 120;
    constexpr std::size_t chunk_size = 1;
    constexpr std::size_t num_chunks = 120;

    EXT_ALLOCATOR_NAMESPACE::
        bitmap_allocator<EXT_ALLOCATOR_NAMESPACE::blob_allocator<alignment, size>, alignment, chunk_size, num_chunks>
            a;
    auto block1 = a.allocate(alignment, 62);
    EXPECT_EQ(a._free_blocks[0], 0xc000000000000000);
    EXPECT_EQ(a._free_blocks[1], 0x00ffffffffffffff);

    auto block2 = a.allocate(alignment, 4);
    EXPECT_EQ(a._free_blocks[0], 0x0000000000000000);
    EXPECT_EQ(a._free_blocks[1], 0x00fffffffffffffc);

    a.deallocate(block1);
    a.deallocate(block2);
}

TEST(bitmap, test_rejected_sizes) {
    EXT_ALLOCATOR_NAMESPACE::bitmap_allocator<EXT_ALLOCATOR_NAMESPACE::blob_allocator<256, 1>, 1, 256, 1> a;
    auto block = a.allocate(129, 1);
    EXPECT_TRUE(block.data == nullptr);
    EXPECT_EQ(block.size, 0);

    block = a.allocate(512, 1);
    EXPECT_TRUE(block.data == nullptr);
    EXPECT_EQ(block.size, 0);
}

TEST(bitmap, test_chunk_alignment) {
    EXT_ALLOCATOR_NAMESPACE::bitmap_allocator<EXT_ALLOCATOR_NAMESPACE::blob_allocator<8, 128>, 8, 1, 128> a;
    auto block1 = a.allocate(1, 4);
    EXPECT_TRUE(block1.data != nullptr);
    EXPECT_EQ(a._free_blocks[0], 0xfffffffffffffff0);
    EXPECT_EQ(a._free_blocks[1], 0xffffffffffffffff);

    auto block2 = a.allocate(8, 8);
    EXPECT_TRUE(block2.data != nullptr);
    EXPECT_EQ(a._free_blocks[0], 0xffffffffffff00f0);
    EXPECT_EQ(a._free_blocks[1], 0xffffffffffffffff);

    a.deallocate(block1);
    a.deallocate(block2);
}
