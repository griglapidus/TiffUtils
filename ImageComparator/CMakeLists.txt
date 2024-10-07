cmake_minimum_required(VERSION 3.16)

project(ImageComparator VERSION 0.1 LANGUAGES CXX)

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
    qt_add_executable(ImageComparator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ImageComparator APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ImageComparator SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ImageComparator
            ${PROJECT_SOURCES}
        )
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../OpenCV/include)

target_link_libraries(ImageComparator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    debug ${CMAKE_CURRENT_SOURCE_DIR}/../OpenCV/lib/opencv_world4100d.lib
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/../OpenCV/lib/opencv_world4100.lib
    general  ${CMAKE_CURRENT_SOURCE_DIR}/../OpenCV/lib/opencv_world4100.lib)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ImageComparator)
endif()
set_target_properties(ImageComparator PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ImageComparator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
add_custom_command(TARGET ImageComparator POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../OpenCV/bin/opencv_world4100d.dll>
$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/../OpenCV/bin/opencv_world4100.dll>
$<TARGET_FILE_DIR:ImageComparator>)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ImageComparator)
endif()
