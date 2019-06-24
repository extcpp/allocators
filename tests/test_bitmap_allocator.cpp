#include <gtest/gtest.h>

#include <ext/allocators/blob.hpp>

#define private public //geil!!! ballern!!
#include <ext/allocators/bitmap.hpp>
#undef private

TEST(bitmap, test_allocate) {
    alloc::bitmap_allocator<alloc::blob_allocator<64, 8>, 4, 16, 8> a;
    EXPECT_TRUE(a._block.data == nullptr);
    EXPECT_EQ(a._block.size, 0);

    auto block = a.allocate(32, 8);
    EXPECT_EQ(block.size, 32);
    EXPECT_EQ(reinterpret_cast<intptr_t>(block.data) % 8, 0);
    EXPECT_EQ(a._free_blocks[0], 0xff00);

    auto block2 = a.allocate(32, 8);
    EXPECT_EQ(block2.size, 32);
    EXPECT_TRUE(block2.data != block.data);
    EXPECT_EQ(a._free_blocks[0], 0x0);

    a.deallocate(block);
    EXPECT_EQ(a._free_blocks[0], 0xff);

    a.deallocate(block2);
    EXPECT_EQ(a._free_blocks[0], 0xffff);
}

TEST(bitmap, test_split_pattern) {
    alloc::bitmap_allocator<alloc::blob_allocator<120, 1>, 1, 120, 1> a;
    auto block1 = a.allocate(62, 1);
    EXPECT_EQ(a._free_blocks[0], 0xc000000000000000);
    EXPECT_EQ(a._free_blocks[1], 0x00ffffffffffffff);

    auto block2 = a.allocate(4, 1);
    EXPECT_EQ(a._free_blocks[0], 0x0000000000000000);
    EXPECT_EQ(a._free_blocks[1], 0x00fffffffffffffc);

    a.deallocate(block1);
    a.deallocate(block2);
}

TEST(bitmap, test_rejected_sizes) {
    alloc::bitmap_allocator<alloc::blob_allocator<256, 1>, 1, 256, 1> a;
    auto block = a.allocate(129, 1);
    EXPECT_TRUE(block.data == nullptr);
    EXPECT_EQ(block.size, 0);

    block = a.allocate(512, 1);
    EXPECT_TRUE(block.data == nullptr);
    EXPECT_EQ(block.size, 0);
}

TEST(bitmap, test_chunk_alignment) {
    alloc::bitmap_allocator<alloc::blob_allocator<128, 8>, 1, 128, 8> a;
    auto block1 = a.allocate(4, 1);
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
