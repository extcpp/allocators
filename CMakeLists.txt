# Copyright - 2015-2019 - Jan Christoph Uhde <Jan@UhdeJC.com>

cmake_minimum_required(VERSION 3.14)
project(extalloc)

function(ext_log)
    message(STATUS "INFO -- " ${ARGV})
endfunction(ext_log)

if(NOT TARGET ext_shared)
    ext_log("trying to add ext target")
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external_libs/libext)
        ext_log("calling add_subdirectory for libext")
        add_subdirectory(external_libs/libext EXCLUDE_FROM_ALL)
    else()
        ext_log("libext directory does not exist")
    endif()
endif()

if(NOT TARGET ext_shared)
    message(FATAL_ERROR "Unable to get target for ext_shared")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/external_libs/libext/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## OPTIONS
option(EXTALLOC_TESTS "build tests" ON)
option(EXTALLOC_WARNINGS "enable warnings" ON)
option(EXTALLOC_EXCEPTIONS "use exceptions" ON) # FIXME - default should be OFF eventually

## general setup and includes
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # EXTCode / VS folders

get_target_property(LIBEXT_SOURCE_DIR ext_shared SOURCE_DIR)
include(ext_cmake_utils)
ext_setup()


if(UNIX)
    set(EXTALLOC_OUTDIR "")
elseif(MSVC)
    ext_log("binary ouput dir: ${EXTALLOC_OUTDIR}")

    #TODO - move settings below into corresponding targets
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${EXTALLOC_OUTDIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${EXTALLOC_OUTDIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXTALLOC_OUTDIR}")
    set(CMAKE_WINDOWS_EEXTPORT_ALL_SYMBOLS TRUE)
else()
endif()

### define header only lib
add_library(extalloc INTERFACE)
target_include_directories(extalloc INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(extalloc INTERFACE cxx_std_17)
target_compile_options(extalloc INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${EXTALLOC_WARNINGS}>>:${ext_stone-warnings}>
)

target_compile_definitions(extalloc INTERFACE
    $<$<BOOL:${EXTALLOC_EXCEPTIONS}>:EXTALLOC_EXCEPTIONS=1>
)

target_compile_definitions(extalloc INTERFACE
    ${ext_common_private_compile_definitions}
    EXTALLOC_HEADER_ONLY_LIB
)

if(EXTALLOC_TESTS AND TARGET ext)
    ext_log("extalloc tests enabled")
    include(CTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ext_add_test_subdirectory("google" ${CMAKE_CURRENT_SOURCE_DIR}/tests)
else()
    ext_log("extalloc tests disabled")
endif()
