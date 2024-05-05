cmake_minimum_required(VERSION 3.5)

project(TiffGrayscaleConverter)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
set(ZLIB_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/libzlib.dll.a)
set(ZLIB_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

# Include subdirectories
add_subdirectory(zlib)
add_subdirectory(LibTiff)
add_subdirectory(TiffGrayscaleConverter)
add_subdirectory(TiffTagEditor)
