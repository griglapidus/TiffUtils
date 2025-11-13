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

set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON)
set(zlib_DIR ${zlib_SOURCE_DIR})

FetchContent_MakeAvailable(zlib tiff)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(TiffGrayscaleConverter)
add_subdirectory(TiffTagEditor)
# add_subdirectory(ImageComparator)
