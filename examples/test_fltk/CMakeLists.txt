find_package(FLTK REQUIRED)
find_package(OpenCV REQUIRED core imgproc imgcodecs videoio)

add_executable(test_fltk test_fltk.cpp)

target_include_directories(test_fltk PRIVATE ${FLTK_INCLUDE_DIR})
target_link_libraries(test_fltk PRIVATE
    ${FLTK_LIBRARIES}
    ${OpenCV_LIBS}
) 