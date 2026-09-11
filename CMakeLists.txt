cmake_minimum_required(VERSION 3.24)
project(DrPdf VERSION 1.0.0 LANGUAGES CXX)

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
    Svg
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

include(CheckCXXSourceCompiles)
get_target_property(_qpdf_incs ${DRPDF_QPDF_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
set(CMAKE_REQUIRED_INCLUDES ${_qpdf_incs} ${QPDF_INCLUDE})
check_cxx_source_compiles("
#include <qpdf/Constants.h>
int main() { auto x = qpdf_r3p_full; (void)x; return 0; }
" DRPDF_HAS_QPDF_R3P)
if(DRPDF_HAS_QPDF_R3P)
    add_compile_definitions(DRPDF_QPDF_R3P=1)
endif()

add_subdirectory(src)
add_subdirectory(probe)

if(DRPDF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)
install(TARGETS DrPdf
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(UNIX AND NOT APPLE)
    install(FILES packaging/linux/drpdf.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES packaging/linux/drpdf.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
endif()

set(CPACK_PACKAGE_NAME "DrPdf")
set(CPACK_PACKAGE_VENDOR "Dr PDF")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Local-first offline PDF editor")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_HOMEPAGE_URL "https://drpdf.app")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Dr PDF")
    set(CPACK_NSIS_PACKAGE_NAME "Dr PDF")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Dr PDF")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dr PDF")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6, libqt6gui6, libqt6pdf6, libqpdf29")
endif()
include(CPack)
