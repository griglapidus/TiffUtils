cmake_minimum_required(VERSION 3.5)

project(TiffGrayScaleConverter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TiffGrayScaleConverter
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        TiffConverter.h TiffConverter.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TiffGrayScaleConverter APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TiffGrayScaleConverter SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TiffGrayScaleConverter
            ${PROJECT_SOURCES}
        )
    endif()
endif()

include_directories(LibZip)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBZIP_DIR ${CMAKE_SOURCE_DIR}/LibZip/Debug64)
else()
    set(LIBZIP_DIR ${CMAKE_SOURCE_DIR}/LibZip/Release64)
endif()

include_directories(LibTiff)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBTIFF_DIR ${CMAKE_SOURCE_DIR}/LibTiff/debug64)
else()
    set(LIBTIFF_DIR ${CMAKE_SOURCE_DIR}/LibTiff/release64)
endif()

target_link_libraries(TiffGrayScaleConverter PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${LIBZIP_DIR}/Libzip.lib ${LIBTIFF_DIR}/LibTiff.lib)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
else()
    message(FATAL_ERROR "AVX2 is not supported with this compiler")
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TiffGrayScaleConverter)
endif()
set_target_properties(TiffGrayScaleConverter PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TiffGrayScaleConverter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TiffGrayScaleConverter)
endif()
