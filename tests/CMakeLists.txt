add_executable(drpdf-tests
    test_assembly.cpp
    test_ranges.cpp
)
target_link_libraries(drpdf-tests PRIVATE drpdf-core)
target_include_directories(drpdf-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME drpdf-core COMMAND drpdf-tests)
