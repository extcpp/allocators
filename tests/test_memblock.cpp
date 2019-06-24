#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE memblock
#include <boost/test/unit_test.hpp>

#include "memblock.hpp"

#include "singleton_allocator.hpp"
#define private public
#include "blob_allocator.hpp"
#undef private

BOOST_AUTO_TEST_CASE(test_deleter) {
    using allocator_t = alloc::singleton_allocator<alloc::blob_allocator<4, 4>>;

    {
        auto up = alloc::make_unique<int, allocator_t>(nullptr, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        BOOST_CHECK((std::is_same<deleter_t, alloc::deleter<allocator_t>>::value));

        BOOST_CHECK_EQUAL(*up, 42);
        BOOST_CHECK_EQUAL(sizeof(up), sizeof(void*));
        BOOST_CHECK_EQUAL(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(allocator_t::instance()._data));
        BOOST_CHECK_EQUAL(allocator_t::instance()._allocated, true);
    }
    BOOST_CHECK_EQUAL(allocator_t::instance()._allocated, false);
}

BOOST_AUTO_TEST_CASE(test_deleter_divergent_size) {
    using allocator_t = alloc::singleton_allocator<alloc::blob_allocator<64, 4>>;

    {
        auto up = alloc::make_unique<int, allocator_t>(nullptr, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        BOOST_CHECK(
            (std::is_same<deleter_t, alloc::deleter<allocator_t, alloc::deleter_options::divergent_size>>::value));

        BOOST_CHECK_EQUAL(*up, 42);
        BOOST_CHECK_EQUAL(sizeof(up), sizeof(void*) * 2);
        BOOST_CHECK_EQUAL(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(allocator_t::instance()._data));
        BOOST_CHECK_EQUAL(allocator_t::instance()._allocated, true);
    }
    BOOST_CHECK_EQUAL(allocator_t::instance()._allocated, false);
}

BOOST_AUTO_TEST_CASE(test_deleter_local) {
    using allocator_t = alloc::blob_allocator<4, 4>;

    allocator_t a;
    {
        auto up = alloc::make_unique<int>(&a, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        BOOST_CHECK((std::is_same<deleter_t, alloc::deleter<allocator_t, alloc::deleter_options::local>>::value));

        BOOST_CHECK_EQUAL(*up, 42);
        BOOST_CHECK_EQUAL(sizeof(up), sizeof(void*) * 2);
        BOOST_CHECK_EQUAL(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(a._data));
        BOOST_CHECK_EQUAL(a._allocated, true);
    }
    BOOST_CHECK_EQUAL(a._allocated, false);
}

BOOST_AUTO_TEST_CASE(test_deleter_divsize_local) {
    using allocator_t = alloc::blob_allocator<64, 4>;

    allocator_t a;
    {
        auto up = alloc::make_unique<int>(&a, 42);

        using deleter_t = std::remove_reference_t<decltype(up)>::deleter_type;
        BOOST_CHECK((std::is_same<
                     deleter_t,
                     alloc::deleter<allocator_t,
                                    alloc::deleter_options::divergent_size | alloc::deleter_options::local>>::value));

        BOOST_CHECK_EQUAL(*up, 42);
        BOOST_CHECK_EQUAL(sizeof(up), sizeof(void*) * 3);
        BOOST_CHECK_EQUAL(reinterpret_cast<void*>(up.get()), reinterpret_cast<void*>(a._data));
        BOOST_CHECK_EQUAL(a._allocated, true);
    }
    BOOST_CHECK_EQUAL(a._allocated, false);
}
