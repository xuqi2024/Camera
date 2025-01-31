/**
 * @file test_fltk.cpp
 * @brief FLTK视频显示测试程序
 */
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <opencv2/opencv.hpp>

class CameraWindow : public Fl_Window {
public:
    CameraWindow(int w, int h, const char* title) 
        : Fl_Window(w, h, title) {
        
        box = new Fl_Box(0, 0, w, h);
        capture.open(0);
        
        if (!capture.isOpened()) {
            fl_alert("无法打开摄像头");
            return;
        }
        
        Fl::add_timeout(1.0/30.0, timer_cb, this);
    }
    
    ~CameraWindow() {
        capture.release();
    }
    
    static void timer_cb(void* v) {
        CameraWindow* win = (CameraWindow*)v;
        win->update_frame();
        Fl::repeat_timeout(1.0/30.0, timer_cb, v);
    }
    
    void update_frame() {
        cv::Mat frame;
        if (capture.read(frame)) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            Fl_RGB_Image img(frame.data, frame.cols, frame.rows, 3);
            box->image(img.copy());
            box->redraw();
        }
    }
    
private:
    Fl_Box* box;
    cv::VideoCapture capture;
};

int main(int argc, char *argv[]) {
    CameraWindow* window = new CameraWindow(640, 480, "FLTK摄像头查看器");
    window->show(argc, argv);
    return Fl::run();
} 