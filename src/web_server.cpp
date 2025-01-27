#include "web_server.h"
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <sstream>

using namespace Poco::Net;

WebServer::RequestHandler::RequestHandler(std::shared_ptr<CaptureInterface> capture)
    : video_capture_(capture) {}

void WebServer::RequestHandler::handleRequest(HTTPServerRequest& request, 
                                           HTTPServerResponse& response) {
    if (request.getURI() == "/") {
        response.setContentType("text/html");
        std::ostream& out = response.send();
        out << R"(
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
    } else if (request.getURI() == "/stream") {
        response.setContentType("image/jpeg");
        std::string frame = video_capture_->getLatestFrame();
        response.sendBuffer(frame.data(), frame.size());
    } else {
        response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        response.send();
    }
}

WebServer::RequestHandlerFactory::RequestHandlerFactory(
    std::shared_ptr<CaptureInterface> capture)
    : video_capture_(capture) {}

HTTPRequestHandler* WebServer::RequestHandlerFactory::createRequestHandler(
    const HTTPServerRequest&) {
    return new RequestHandler(video_capture_);
}

WebServer::WebServer(std::shared_ptr<CaptureInterface> video_capture)
    : video_capture_(video_capture) {}

void WebServer::start(int port) {
    auto* params = new HTTPServerParams;
    params->setMaxQueued(100);
    params->setMaxThreads(4);

    ServerSocket socket(port);
    server_ = std::make_unique<HTTPServer>(
        new RequestHandlerFactory(video_capture_), socket, params);
    server_->start();
}

void WebServer::stop() {
    if (server_) {
        server_->stop();
        server_.reset();
    }
} 