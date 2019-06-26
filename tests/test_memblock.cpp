#include <gtest/gtest.h>

#include <ext/allocators/detail_block.hpp>

#include <ext/allocators/singleton.hpp>
#define private public
#include <ext/allocators/blob.hpp>
#include <ext/allocators/stl_wrapper.hpp>
#undef private

TEST(block, test_deleter) {
    using allocator_t = ext::allocoators::singleton_allocator<ext::allocoators::blob_allocator<4, 4>>;

    {
        auto up = ext::allocoators::make_unique<int, allocator_t>(nullptr, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE((std::is_same<deleter_t, ext::allocoators::deleter<allocator_t>>::value));
        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*));
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(allocator_t::instance()._data));
        EXPECT_EQ(allocator_t::instance()._allocated, true);
    }
    EXPECT_EQ(allocator_t::instance()._allocated, false);
}

TEST(block, test_deleter_divergent_size) {
    using allocator_t = ext::allocoators::singleton_allocator<ext::allocoators::blob_allocator<4, 64>>;

    {
        auto up = ext::allocoators::make_unique<int, allocator_t>(nullptr, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE(
            (std::is_same<
                deleter_t,
                ext::allocoators::deleter<allocator_t, ext::allocoators::deleter_options::divergent_size>>::value));

        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*) * 2);
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(allocator_t::instance()._data));
        EXPECT_EQ(allocator_t::instance()._allocated, true);
    }
    EXPECT_EQ(allocator_t::instance()._allocated, false);
}

TEST(block, test_deleter_local) {
    using allocator_t = ext::allocoators::blob_allocator<4, 4>;

    allocator_t a;
    {
        auto up = ext::allocoators::make_unique<int>(&a, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE(
            (std::is_same<deleter_t,
                          ext::allocoators::deleter<allocator_t, ext::allocoators::deleter_options::local>>::value));

        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*) * 2);
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(a._data));
        EXPECT_EQ(a._allocated, true);
    }
    EXPECT_EQ(a._allocated, false);
}

TEST(block, test_deleter_divsize_local) {
    using allocator_t = ext::allocoators::blob_allocator<4, 64>;

    allocator_t a;
    {
        auto up = ext::allocoators::make_unique<int>(&a, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE((std::is_same<deleter_t,
                                  ext::allocoators::deleter<allocator_t,
                                                            ext::allocoators::deleter_options::divergent_size |
                                                                ext::allocoators::deleter_options::local>>::value));

        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*) * 3);
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(a._data));
        EXPECT_EQ(a._allocated, true);
    }
    EXPECT_EQ(a._allocated, false);
}
