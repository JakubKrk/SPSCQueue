add_executable(spscringbuffer_tests
    spscringbuffer_tests.cpp
)

target_link_libraries(spscringbuffer_tests
    spscringbuffer
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(spscringbuffer_tests)
