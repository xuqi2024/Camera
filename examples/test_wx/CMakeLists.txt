find_package(wxWidgets REQUIRED core base)
find_package(OpenCV REQUIRED core imgproc imgcodecs videoio)

include(${wxWidgets_USE_FILE})

add_executable(test_wx test_wx.cpp)

target_link_libraries(test_wx PRIVATE
    ${wxWidgets_LIBRARIES}
    ${OpenCV_LIBS}
) 