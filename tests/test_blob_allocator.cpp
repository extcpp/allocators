#include <gtest/gtest.h>

#define private public
#include <ext/allocators/blob.hpp>
#undef private

TEST(blob, test_allocate) {
    alloc::blob_allocator<64, 8> a;

    auto block = a.allocate(64, 8);
    EXPECT_EQ(block.size, 64);
    EXPECT_EQ(reinterpret_cast<intptr_t>(block.data) % 8, 0);
    EXPECT_EQ(a._allocated, true);

    auto block2 = a.allocate(64, 8);
    EXPECT_TRUE(block2.data == nullptr);
    EXPECT_EQ(block2.size, 0);
    EXPECT_EQ(a._allocated, true);

    a.deallocate(block);
    EXPECT_EQ(a._allocated, false);

    block = a.allocate(64, 8);
    EXPECT_EQ(block.size, 64);
    EXPECT_EQ(reinterpret_cast<intptr_t>(block.data) % 8, 0);
    EXPECT_EQ(a._allocated, true);

    a.deallocate(block);
    EXPECT_EQ(a._allocated, false);
}
