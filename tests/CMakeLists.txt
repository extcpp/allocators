project(libext-tests)

set(test_files 
    "test_allocator_wrapper"
    "test_bitmap_allocator"
    "test_blob_allocator"
    "test_memblock"
)

#build one executable
set(test_sources)
foreach(test_name IN LISTS test_files) # <- DO NOT EXPAND LIST
    list(APPEND test_sources "${test_name}.cpp")
endforeach()

#define test target
add_executable(test_extalloc gtest.cpp ${test_sources})
target_include_directories(test_extalloc SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include)
target_link_libraries(test_extalloc extalloc gtest_main gtest)
target_compile_options(test_extalloc PRIVATE ${ext_stone-warnings})

#add test to ctest
add_test(NAME test_extalloc_run COMMAND $<TARGET_FILE:test_extalloc>)

#ui
set_target_properties (test_extalloc PROPERTIES FOLDER tests/test_extalloc)
