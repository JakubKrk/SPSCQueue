add_executable(spscringbuffer_tests
    spscringbuffer_tests.cpp
)

target_link_libraries(spscringbuffer_tests
    spscringbuffer
    GTest::gtest_main
)

add_executable(spscringbuffer_thread_tests
    spscringbuffer_thread_tests.cpp
)

target_link_libraries(spscringbuffer_thread_tests
    spscringbuffer
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(spscringbuffer_tests)
gtest_discover_tests(spscringbuffer_thread_tests)
