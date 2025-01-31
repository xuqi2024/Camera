find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
find_package(OpenCV REQUIRED core imgproc imgcodecs videoio)

add_executable(test_gtk test_gtk.cpp)

target_include_directories(test_gtk PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(test_gtk PRIVATE
    ${GTK3_LIBRARIES}
    ${OpenCV_LIBS}
) 