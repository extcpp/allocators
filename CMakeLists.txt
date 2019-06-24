# Copyright - 2015-2019 - Jan Christoph Uhde <Jan@UhdeJC.com>

cmake_minimum_required(VERSION 3.8)
project(extalloc)

if(NOT TARGET extbase)
    add_subdirectory(external_libs/base EXCLUDE_FROM_ALL)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/external_libraries/base/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## OPTIONS
#option(EXTALLOC_EEXTAMPLES "build examples" OFF)
option(EXTALLOC_TESTS "build tests" ON)
option(EXTALLOC_WARNINGS "enable warnings" ON)

## general setup and includes
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # EXTCode / VS folders

include(ext_cmake_utils)
ext_setup()

function(ext_log)
    message(STATUS "INFO -- " ${ARGV}) 
endfunction(ext_log)

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

# required by folder structure for EXTCode and VisualStudio (includes)
# sources are always required
#include(src_files.cmake)
#include(include_files.cmake)

#### library setup
#set(ext_common_private_compile_definitions
#    $<$<BOOL:${EXTALLOC_CEXTEXT_COMPILER_IS_GCC}>:EXTALLOC_GCC>
#    $<$<BOOL:${EXTALLOC_CEXTEXT_COMPILER_IS_CLANG}>:EXTALLOC_CLANG>
#    EXTALLOC_IN_LIB=1
#)

### define header only lib
add_library(extalloc INTERFACE)
target_include_directories(extalloc INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(extalloc INTERFACE cxx_std_17)
target_compile_options(extalloc INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<BOOL:EXTALLOC_WARNINGS>>:${ext_stone-warnings}>
)

target_compile_definitions(extalloc INTERFACE
    ${ext_common_private_compile_definitions}
    EXTALLOC_HEADER_ONLY_LIB
)

# set up folder structure for EXTCode and VisualStudio
#set_target_properties (extalloc PROPERTIES FOLDER extalloc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${libext_header} ${libext_source})

if(EXTALLOC_TESTS)
    ext_log("extalloc tests enabled")
    include(CTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ext_add_test_subdirectory("google" tests)
else()
    ext_log("extalloc tests disabled")
endif()

## add projects using this lib
if(EXTALLOC_EAMPLES)
    ext_log("extalloc examples enabled")
    add_subdirectory(examples)
else()
    ext_log("extalloc examples disabled")
endif()

## installation
#if(COMMAND ext_install)
#    ext_install(extalloc "include/ext")
#endif()
