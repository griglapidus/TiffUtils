cmake_minimum_required(VERSION 3.5)

project(TiffGrayscaleConverter)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.1
)

FetchContent_Declare(
    tiff
    GIT_REPOSITORY https://gitlab.com/libtiff/libtiff.git
    GIT_TAG        v4.7.1
)

set(ZLIB_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(tiff-tests OFF CACHE INTERNAL "Disable LibTiff tests")
set(tiff-tools OFF CACHE INTERNAL "Disable LibTiff tools")
set(tiff-contrib OFF CACHE INTERNAL "Disable LibTiff contrib")

# Build zlib first so we can wire it into libtiff's find_package(ZLIB).
FetchContent_MakeAvailable(zlib)

# zlib's CMake creates "zlib" (shared) and "zlibstatic" (static) targets but no
# ZLIB::ZLIB alias, and it does NOT register as a found package. libtiff calls
# the stock find_package(ZLIB), which only looks for a system-installed zlib.
# Expose the FetchContent build under the names FindZLIB / libtiff expect so the
# Deflate (AdobeDeflate) codec is compiled in (sets ZIP_SUPPORT).
if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

set(ZLIB_INCLUDE_DIR     "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE STRING "" FORCE)
set(ZLIB_INCLUDE_DIRS    "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE STRING "" FORCE)
set(ZLIB_LIBRARY_RELEASE zlibstatic CACHE STRING "" FORCE)
set(ZLIB_LIBRARY         zlibstatic CACHE STRING "" FORCE)
set(ZLIB_LIBRARIES       zlibstatic CACHE STRING "" FORCE)
set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)

# Force libtiff's zlib option ON (overrides any stale cached value).
set(zlib ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(tiff)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(TiffGrayscaleConverter)
add_subdirectory(TiffTagEditor)
# add_subdirectory(ImageComparator)
