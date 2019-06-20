# Copyright - 2015-2019 - Jan Christoph Uhde <Jan@UhdeJC.com>

cmake_minimum_required(VERSION 3.8)
project(xalloc)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## OPTIONS
option(XALLOC_EXAMPLES "build examples" OFF)
option(XALLOC_TESTS "build tests" OFF)
option(XALLOC_WARNINGS "enable warnings" ON)

## general setup and includes
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # XCode / VS folders

#include(ext_cmake_utils)
#ext_setup()

function(x_log)
    message(STATUS "INFO -- " ${ARGV}) 
endfunction(x_log)

if(UNIX)
    set(XALLOC_OUTDIR "")
elseif(MSVC)
    x_log("binary ouput dir: ${XALLOC_OUTDIR}")

    #TODO - move settings below into corresponding targets
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${XALLOC_OUTDIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${XALLOC_OUTDIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${XALLOC_OUTDIR}")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
else()
endif()

# required by folder structure for XCode and VisualStudio (includes)
# sources are always required
#include(src_files.cmake)
#include(include_files.cmake)

#### library setup
#set(ext_common_private_compile_definitions
#    $<$<BOOL:${XALLOC_CXX_COMPILER_IS_GCC}>:XALLOC_GCC>
#    $<$<BOOL:${XALLOC_CXX_COMPILER_IS_CLANG}>:XALLOC_CLANG>
#    XALLOC_IN_LIB=1
#)

### define header only lib
add_library(xalloc INTERFACE)
target_include_directories(xalloc INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(xalloc INTERFACE cxx_std_17)
target_compile_options(xalloc INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<BOOL:XALLOC_WARNINGS>>:${ext_stone-warnings}>
)

target_compile_definitions(xalloc INTERFACE
    ${ext_common_private_compile_definitions}
    XALLOC_HEADER_ONLY_LIB
)

# set up folder structure for XCode and VisualStudio
set_target_properties (alloc PROPERTIES FOLDER alloc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${libext_header} ${libext_source})

## testing
if(XALLOC_TESTS)
    x_log("libxalloc tests enabled")
    include(CTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    #ext_add_test_subdirectory("google" tests)
else()
    x_log("libxalloc tests disabled")
endif()

## add projects using this lib
if(XALLOC_EXAMPLES)
    x_log("libxalloc examples enabled")
    add_subdirectory(examples)
else()
    x_log("libxalloc examples disabled")
endif()

## installation
#if(COMMAND ext_install)
#    ext_install(xalloc "include/ext")
#endif()
