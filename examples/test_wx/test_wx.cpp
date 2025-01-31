/**
 * @file test_wx.cpp
 * @brief wxWidgets视频显示测试程序
 */
#include <wx/wx.h>
#include <opencv2/opencv.hpp>

class CameraPanel : public wxPanel {
public:
    CameraPanel(wxFrame* parent) : wxPanel(parent) {
        capture.open(0);
        if (!capture.isOpened()) {
            wxMessageBox("无法打开摄像头", "错误");
            return;
        }
        
        timer = new wxTimer(this);
        Bind(wxEVT_TIMER, &CameraPanel::OnTimer, this);
        timer->Start(33); // ~30 FPS
        
        Bind(wxEVT_PAINT, &CameraPanel::OnPaint, this);
    }
    
    ~CameraPanel() {
        timer->Stop();
        delete timer;
        capture.release();
    }
    
private:
    void OnTimer(wxTimerEvent& event) {
        cv::Mat frame;
        if (capture.read(frame)) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            wxImage img(frame.cols, frame.rows, frame.data, true);
            bitmap = wxBitmap(img);
            Refresh();
        }
    }
    
    void OnPaint(wxPaintEvent& event) {
        wxPaintDC dc(this);
        if (bitmap.IsOk()) {
            dc.DrawBitmap(bitmap, 0, 0, false);
        }
    }
    
    cv::VideoCapture capture;
    wxTimer* timer;
    wxBitmap bitmap;
};

class MainFrame : public wxFrame {
public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets摄像头查看器") {
        SetClientSize(640, 480);
        new CameraPanel(this);
    }
};

class App : public wxApp {
public:
    bool OnInit() {
        MainFrame* frame = new MainFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(App); 