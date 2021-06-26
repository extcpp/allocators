#include <gtest/gtest.h>

#include <ext/allocators/memory_block.hpp>
#include <ext/allocators/singleton.hpp>

#define private public
#include <ext/allocators/blob.hpp>
#include <ext/allocators/stl_wrapper.hpp>
#undef private

TEST(block, test_deleter) {
    using allocator_t = EXT_ALLOCATOR_NAMESPACE::singleton_allocator<EXT_ALLOCATOR_NAMESPACE::blob_allocator<4, 4>>;

    {
        auto up = EXT_ALLOCATOR_NAMESPACE::make_unique<int, allocator_t>(nullptr, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE((std::is_same<deleter_t, EXT_ALLOCATOR_NAMESPACE::deleter<allocator_t>>::value));
        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*));
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(allocator_t::instance()._data));
        EXPECT_EQ(allocator_t::instance()._allocated, true);
    }
    EXPECT_EQ(allocator_t::instance()._allocated, false);
}

TEST(block, test_deleter_divergent_size) {
    using allocator_t = EXT_ALLOCATOR_NAMESPACE::singleton_allocator<EXT_ALLOCATOR_NAMESPACE::blob_allocator<4, 64>>;

    {
        auto up = EXT_ALLOCATOR_NAMESPACE::make_unique<int, allocator_t>(nullptr, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE(
            (std::is_same<deleter_t,
                          EXT_ALLOCATOR_NAMESPACE::
                              deleter<allocator_t, EXT_ALLOCATOR_NAMESPACE::deleter_options::divergent_size>>::value));

        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*) * 2);
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(allocator_t::instance()._data));
        EXPECT_EQ(allocator_t::instance()._allocated, true);
    }
    EXPECT_EQ(allocator_t::instance()._allocated, false);
}

TEST(block, test_deleter_local) {
    using allocator_t = EXT_ALLOCATOR_NAMESPACE::blob_allocator<4, 4>;

    allocator_t a;
    {
        auto up = EXT_ALLOCATOR_NAMESPACE::make_unique<int>(&a, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE(
            (std::is_same<deleter_t,
                          EXT_ALLOCATOR_NAMESPACE::deleter<allocator_t,
                                                           EXT_ALLOCATOR_NAMESPACE::deleter_options::local>>::value));

        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*) * 2);
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(a._data));
        EXPECT_EQ(a._allocated, true);
    }
    EXPECT_EQ(a._allocated, false);
}

TEST(block, test_deleter_divsize_local) {
    using allocator_t = EXT_ALLOCATOR_NAMESPACE::blob_allocator<4, 64>;

    allocator_t a;
    {
        auto up = EXT_ALLOCATOR_NAMESPACE::make_unique<int>(&a, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        EXPECT_TRUE((std::is_same<
                     deleter_t,
                     EXT_ALLOCATOR_NAMESPACE::deleter<allocator_t,
                                                      EXT_ALLOCATOR_NAMESPACE::deleter_options::divergent_size |
                                                          EXT_ALLOCATOR_NAMESPACE::deleter_options::local>>::value));

        EXPECT_EQ(*up, 42);
        EXPECT_EQ(sizeof(up), sizeof(void*) * 3);
        EXPECT_EQ(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(a._data));
        EXPECT_EQ(a._allocated, true);
    }
    EXPECT_EQ(a._allocated, false);
}
