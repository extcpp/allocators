#include <gtest/gtest.h>

#define private public
#include <ext/allocators/blob.hpp>
#undef private

TEST(blob, test_allocate) {

    // allocate in memory of 64bit with alignment of 8bit
    EXT_ALLOCATOR_NAMESPACE::blob_allocator<8, 64> a;

    auto block = a.allocate(8, 64);
    EXPECT_EQ(block.size, 64);
    EXPECT_EQ(reinterpret_cast<intptr_t>(block.data) % 8, 0);
    EXPECT_EQ(a._allocated, true);

    auto block2 = a.allocate(8, 64);
    EXPECT_TRUE(block2.data == nullptr);
    EXPECT_EQ(block2.size, 0);
    EXPECT_EQ(a._allocated, true);

    a.deallocate(block);
    EXPECT_EQ(a._allocated, false);

    block = a.allocate(8, 64);
    EXPECT_EQ(block.size, 64);
    EXPECT_EQ(reinterpret_cast<intptr_t>(block.data) % 8, 0);
    EXPECT_EQ(a._allocated, true);

    a.deallocate(block);
    EXPECT_EQ(a._allocated, false);
}
