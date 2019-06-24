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

set(test_target "test_extalloc")
add_executable("${test_target}" gtest.cpp ${test_sources})
target_include_directories("${test_target}" SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include)
target_link_libraries("${test_target}" extalloc gtest_main gtest)
target_compile_options("${test_target}" PRIVATE ${ext_stone-warnings})
add_test(NAME "${test_target}_run" COMMAND $<TARGET_FILE:${test_target}>)
set_target_properties (${test_target} PROPERTIES FOLDER tests/${test_target})
