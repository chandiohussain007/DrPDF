add_library(drpdf-core STATIC
    core/assembly.cpp
    core/content_util.cpp
    core/text_edit.cpp
    core/overlay.cpp
    core/annotations.cpp
    core/ocr_text.cpp
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
    resources/resources.qrc
    app/settings.cpp
    services/image_pdf.cpp
    services/rich_pdf.cpp
    services/thumbnails.cpp
    services/working_copy.cpp
    services/image_compress.cpp
    services/ocr_engine.cpp
    services/pdf_sign.cpp
    ui/theme/theme.cpp
    ui/icons/icons.cpp
    ui/shell/main_window.cpp
    ui/shell/sidebar.cpp
    ui/shell/title_bar.cpp
    ui/shell/menu_bar.cpp
    ui/shell/ribbon_bar.cpp
    ui/shell/icon_rail.cpp
    ui/shell/status_bar.cpp

    ui/components/drop_zone.cpp
    ui/components/tool_card.cpp
    ui/components/page_grid.cpp
    ui/components/banner.cpp
    ui/components/page_canvas.cpp
    ui/components/signature_pad.cpp
    ui/components/pages_dock.cpp
    ui/components/inspector_dock.cpp
    ui/components/assembly_dock.cpp
    ui/components/optimization_dock.cpp
    ui/components/security_dock.cpp
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
    ui/views/edit_view.cpp
    ui/views/annotate_view.cpp
    ui/views/watermark_view.cpp
    ui/views/ocr_view.cpp
    ui/views/sign_view.cpp
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
    Qt6::Svg
    Qt6::Pdf
    Qt6::PdfWidgets
    Qt6::Concurrent
    Qt6::PrintSupport
)


target_include_directories(DrPdf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    target_compile_definitions(DrPdf PRIVATE DRPDF_HAS_OPENSSL=1)
    target_link_libraries(DrPdf PRIVATE OpenSSL::Crypto)
    message(STATUS "Dr PDF: OpenSSL found — PKCS#12 digital signatures enabled")
else()
    message(STATUS "Dr PDF: OpenSSL not found — visual signatures only")
endif()


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
