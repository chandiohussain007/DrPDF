# Standalone probe: drives CreateView with synthetic keystrokes to reproduce crashes.
qt_add_executable(drpdf-probe
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/views/create_view.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/views/create_view.h
    ${CMAKE_SOURCE_DIR}/src/ui/components/banner.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/banner.h
    ${CMAKE_SOURCE_DIR}/src/ui/components/drop_zone.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/drop_zone.h
    ${CMAKE_SOURCE_DIR}/src/ui/theme/theme.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/icons/icons.cpp
    ${CMAKE_SOURCE_DIR}/src/app/settings.cpp
    ${CMAKE_SOURCE_DIR}/src/services/rich_pdf.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/resources.qrc
)
target_include_directories(drpdf-probe PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(drpdf-probe PRIVATE
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