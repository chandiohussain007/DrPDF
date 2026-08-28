cmake_minimum_required(VERSION 3.24)
project(DrPdf VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(DRPDF_BUILD_TESTS "Build unit tests for /core" ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Pdf
    PdfWidgets
    Concurrent
    PrintSupport
)


# QPDF — vcpkg (qpdf::libqpdf), CMake package, or pkg-config
set(DRPDF_QPDF_TARGET "")
find_package(qpdf CONFIG QUIET)
if(TARGET qpdf::libqpdf)
    set(DRPDF_QPDF_TARGET qpdf::libqpdf)
elseif(TARGET QPDF::libqpdf)
    set(DRPDF_QPDF_TARGET QPDF::libqpdf)
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBQPDF IMPORTED_TARGET libqpdf)
        if(TARGET PkgConfig::LIBQPDF)
            set(DRPDF_QPDF_TARGET PkgConfig::LIBQPDF)
        endif()
    endif()
endif()

if(DRPDF_QPDF_TARGET STREQUAL "")
    find_library(QPDF_LIB NAMES qpdf libqpdf REQUIRED)
    find_path(QPDF_INCLUDE NAMES qpdf/QPDF.hh REQUIRED)
    add_library(drpdf_qpdf INTERFACE)
    target_include_directories(drpdf_qpdf INTERFACE ${QPDF_INCLUDE})
    target_link_libraries(drpdf_qpdf INTERFACE ${QPDF_LIB})
    set(DRPDF_QPDF_TARGET drpdf_qpdf)
endif()

add_subdirectory(src)

if(DRPDF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
