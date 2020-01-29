# Copyright - 2016-2020 - Jan Christoph Uhde <Jan@UhdeJC.com>

cmake_minimum_required(VERSION 3.14)
project(ext-allocators VERSION 0.0.1 LANGUAGES CXX)
message(STATUS "extINFO -- entering ext-allocators")

## OPTIONS
option(EXTALLOC_EXAMPLES "build examples" OFF)
option(EXTALLOC_WARNINGS "enable warnings" ON)
option(EXTALLOC_CHECKED "user assert" ON)

option(EXTALLOC_TESTS "build tests" OFF)

## general setup and includes
# enable extcpp cmake
include(${CMAKE_CURRENT_LIST_DIR}/ext_cmake_enable.cmake)
include(ext_cmake_setup)
ext_enable_ext_libs(basics)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # XCode / VS folders
find_package(Threads REQUIRED)

# verbose windows linking
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /VERBOSE:LIB")

if(LINUX)
    set(EXT_OUTDIR "")
elseif(MSVC)
    #TODO - move settings below into corresponding targets
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${EXT_OUTDIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${EXT_OUTDIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXT_OUTDIR}")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

# include build dir to find version.hpp
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# required by folder structure for XCode and VisualStudio (includes)
# sources are always required
include(src_files.cmake)
include(include_files.cmake)

add_library(ext-allocators INTERFACE ${ext-allocators-source} ${ext-allocators-header})
target_include_directories(ext-allocators INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

target_compile_features(ext-allocators INTERFACE cxx_std_17)
target_compile_definitions(ext-allocators INTERFACE EXTALLOC_HEADER_ONLY_LIB)
target_compile_definitions(ext-allocators INTERFACE $<$<BOOL:${EXTALLOC_EXCEPTIONS}>:EXTALLOC_EXCEPTIONS=1>)

# set up folder structure for XCode and VisualStudio
# set_target_properties (ext-allocators PROPERTIES FOLDER libext) # not for INTERFACE libs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${libext_header} ${libext_source})

add_library(ext::allocators ALIAS ext-allocators)

## testing
if(EXTALLOC_TESTS)
    ext_log("ext-allocators tests enabled")
    include(CTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ext_add_test_subdirectory("google" tests)
else()
    ext_log("ext-allocators tests disabled")
endif()

## ## add projects using this lib
## if(EXTALLOC_EXAMPLES)
##     ext_log("ext-allocators examples enabled")
##     add_subdirectory(examples)
## else()
##     ext_log("ext-allocators examples disabled")
## endif()

## installation
if(COMMAND ext_install)
    set_target_properties(ext-allocators PROPERTIES EXPORT_NAME allocators)

    ext_install(ext-allocators include/ext)

    install(TARGETS ext-allocators
        EXPORT ext-allocators-targets
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
	install(
        EXPORT ext-allocators-targets
		FILE ext-allocators-config.cmake
        NAMESPACE ext::
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
endif()

add_custom_target(
    update_allocators_version ALL
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND}
                -D "EXT_GIT_VERSION_OUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/ext_version.hpp"
                -P "${ext_cmake_dir}/ext_script_git_version.cmake"
)
