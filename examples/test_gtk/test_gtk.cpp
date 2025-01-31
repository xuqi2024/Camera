/**
 * @file test_gtk.cpp
 * @brief GTK+视频显示测试程序
 */
#include <gtk/gtk.h>
#include <opencv2/opencv.hpp>
#include <cairo.h>
#include <thread>
#include <atomic>

struct AppData {
    GtkWidget *image;
    GtkWidget *window;
    cv::VideoCapture capture;
    std::atomic<bool> running{true};
    cv::Mat frame;
    std::mutex frame_mutex;
};

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppData *app = static_cast<AppData*>(user_data);
    std::lock_guard<std::mutex> lock(app->frame_mutex);
    
    if (!app->frame.empty()) {
        cv::Mat rgb;
        cv::cvtColor(app->frame, rgb, cv::COLOR_BGR2RGB);
        
        cairo_surface_t *surface = cairo_image_surface_create_for_data(
            rgb.data,
            CAIRO_FORMAT_RGB24,
            rgb.cols,
            rgb.rows,
            rgb.step
        );
        
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_paint(cr);
        cairo_surface_destroy(surface);
    }
    
    return FALSE;
}

static void capture_thread(AppData *app) {
    while (app->running) {
        cv::Mat frame;
        if (app->capture.read(frame)) {
            std::lock_guard<std::mutex> lock(app->frame_mutex);
            frame.copyTo(app->frame);
            gtk_widget_queue_draw(app->image);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    AppData app;
    app.capture.open(0);
    if (!app.capture.isOpened()) {
        g_print("无法打开摄像头\n");
        return 1;
    }
    
    // 创建窗口
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "GTK摄像头查看器");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 640, 480);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // 创建绘图区域
    app.image = gtk_drawing_area_new();
    gtk_widget_set_size_request(app.image, 640, 480);
    g_signal_connect(app.image, "draw", G_CALLBACK(on_draw), &app);
    
    gtk_container_add(GTK_CONTAINER(app.window), app.image);
    gtk_widget_show_all(app.window);
    
    // 启动捕获线程
    std::thread capture_th(capture_thread, &app);
    
    // 运行主循环
    gtk_main();
    
    // 清理
    app.running = false;
    if (capture_th.joinable()) {
        capture_th.join();
    }
    
    return 0;
} 