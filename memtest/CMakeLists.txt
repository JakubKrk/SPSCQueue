add_executable(memtest memtest.cpp)

target_link_libraries(memtest PRIVATE spscringbuffer)
