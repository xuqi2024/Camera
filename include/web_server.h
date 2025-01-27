#pragma once
#include "capture_interface.h"
#include <Poco/Net/HTTPServer.h>
#include <memory>

class WebServer {
public:
    WebServer(std::shared_ptr<CaptureInterface> video_capture);
    void start(int port = 8080);
    void stop();

private:
    class RequestHandler : public Poco::Net::HTTPRequestHandler {
    public:
        RequestHandler(std::shared_ptr<CaptureInterface> capture);
        void handleRequest(Poco::Net::HTTPServerRequest& request, 
                         Poco::Net::HTTPServerResponse& response) override;
    private:
        std::shared_ptr<CaptureInterface> video_capture_;
    };

    class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        RequestHandlerFactory(std::shared_ptr<CaptureInterface> capture);
        Poco::Net::HTTPRequestHandler* createRequestHandler(
            const Poco::Net::HTTPServerRequest& request) override;
    private:
        std::shared_ptr<CaptureInterface> video_capture_;
    };

    std::shared_ptr<CaptureInterface> video_capture_;
    std::unique_ptr<Poco::Net::HTTPServer> server_;
}; 