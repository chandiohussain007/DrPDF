add_library(drpdf-core STATIC
    core/assembly.cpp
)
target_include_directories(drpdf-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(drpdf-core PUBLIC ${DRPDF_QPDF_TARGET})
target_compile_features(drpdf-core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(drpdf-core PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(drpdf-core PRIVATE -Wall -Wextra -Wpedantic)
endif()


qt_add_executable(DrPdf
    main.cpp
    app/settings.cpp
    services/image_pdf.cpp
    services/rich_pdf.cpp
    services/thumbnails.cpp
    ui/theme/theme.cpp
    ui/icons/icons.cpp
    ui/shell/main_window.cpp
    ui/components/drop_zone.cpp
    ui/components/tool_card.cpp
    ui/components/page_grid.cpp
    ui/components/banner.cpp
    ui/views/home_view.cpp
    ui/views/merge_view.cpp
    ui/views/split_view.cpp
    ui/views/organize_view.cpp
    ui/views/create_view.cpp
    ui/views/images_view.cpp
    ui/views/protect_view.cpp
    ui/views/compress_view.cpp
    ui/views/viewer_view.cpp
    ui/views/coming_soon_view.cpp
    ui/views/command_palette.cpp
)


set_target_properties(DrPdf PROPERTIES
    OUTPUT_NAME "DrPdf"
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Dr PDF"
    MACOSX_BUNDLE_GUI_IDENTIFIER "app.drpdf.desktop"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

target_link_libraries(DrPdf PRIVATE
    drpdf-core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Pdf
    Qt6::PdfWidgets
    Qt6::Concurrent
    Qt6::PrintSupport
)


target_include_directories(DrPdf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(DrPdf PRIVATE /utf-8)
endif()


if(APPLE)
    target_compile_definitions(DrPdf PRIVATE DRPDF_MACOS=1)
elseif(WIN32)
    target_compile_definitions(DrPdf PRIVATE DRPDF_WINDOWS=1)
else()
    target_compile_definitions(DrPdf PRIVATE DRPDF_LINUX=1)
endif()
