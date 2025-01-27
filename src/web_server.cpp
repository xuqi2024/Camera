#include "web_server.h"

WebServer::WebServer(std::shared_ptr<VideoCapture> video_capture)
    : video_capture_(video_capture) {
    
    // 提供静态HTML页面
    CROW_ROUTE(app_, "/")([](){
        return R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>视频流</title>
            </head>
            <body>
                <img id="stream" style="width: 640px; height: 480px;" />
                <script>
                    const img = document.getElementById('stream');
                    setInterval(() => {
                        img.src = '/stream?' + new Date().getTime();
                    }, 50);
                </script>
            </body>
            </html>
        )";
    });

    // 视频流端点
    CROW_ROUTE(app_, "/stream")
    ([this](const crow::request&, crow::response& res){
        auto frame = video_capture_->getLatestFrame();
        res.set_header("Content-Type", "image/jpeg");
        res.write(frame);
        res.end();
    });
}

void WebServer::start(int port) {
    app_.port(port).run();
} 