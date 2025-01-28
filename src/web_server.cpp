#include "web_server.h"
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/JSON/Object.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>

using namespace Poco::Net;
using namespace std::chrono_literals;

WebServer::WebSocketHandler::WebSocketHandler(
    std::shared_ptr<CaptureInterface> capture,
    std::shared_ptr<ImageProcessor> processor)
    : video_capture_(capture), processor_(processor) {}

void WebServer::WebSocketHandler::handleRequest(
    HTTPServerRequest& request, HTTPServerResponse& response) {
    
    if (request.find("Upgrade") != request.end() 
        && Poco::icompare(request["Upgrade"], "websocket") == 0) {
        WebSocket ws(request, response);
        handleWebSocket(ws);
    } else {
        // 处理普通HTTP请求
        if (request.getURI() == "/") {
            response.setContentType("text/html");
            std::ostream& out = response.send();
            out << R"(
<!DOCTYPE html>
<html>
<head>
    <title>智能视频监控</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }
        .container { display: flex; gap: 20px; }
        .video-container { position: relative; flex: 2; }
        .controls { flex: 1; padding: 20px; background: #f5f5f5; border-radius: 8px; }
        #video-canvas { width: 640px; height: 480px; background: #000; }
        #overlay-canvas { position: absolute; top: 0; left: 0; pointer-events: none; }
        .control-group { margin-bottom: 15px; }
        .btn { 
            padding: 8px 16px;
            background: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        .btn:hover { background: #0056b3; }
        .detection-box {
            position: absolute;
            border: 2px solid red;
            display: flex;
            flex-direction: column;
        }
        .stats { 
            position: absolute; 
            top: 10px; 
            left: 10px; 
            background: rgba(0,0,0,0.7);
            color: white;
            padding: 5px;
            border-radius: 4px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="video-container">
            <canvas id="video-canvas"></canvas>
            <canvas id="overlay-canvas"></canvas>
            <div class="stats">
                <div>FPS: <span id="fps">0</span></div>
                <div>Objects: <span id="object-count">0</span></div>
            </div>
        </div>
        <div class="controls">
            <div class="control-group">
                <h3>Detection Settings</h3>
                <label>
                    <input type="checkbox" id="enable-detection" checked>
                    Enable Object Detection
                </label>
                <br><br>
                <label>
                    Confidence Threshold:
                    <input type="range" id="confidence" min="0" max="100" value="50">
                    <span id="confidence-value">0.50</span>
                </label>
            </div>
            <div class="control-group">
                <h3>Video Controls</h3>
                <button id="snapshot" class="btn">Take Snapshot</button>
                <button id="record" class="btn">Start Recording</button>
            </div>
            <div class="control-group">
                <h3>Detection Results</h3>
                <div id="detection-list"></div>
            </div>
        </div>
    </div>
    <script>
        const videoCanvas = document.getElementById('video-canvas');
        const overlayCanvas = document.getElementById('overlay-canvas');
        const ctx = videoCanvas.getContext('2d');
        const overlayCtx = overlayCanvas.getContext('2d');
        
        let ws = null;
        let frameCount = 0;
        let lastTime = performance.now();
        let recording = false;
        let mediaRecorder = null;
        let recordedChunks = [];
        
        function connectWebSocket() {
            ws = new WebSocket(`ws://${location.host}/ws`);
            ws.binaryType = 'arraybuffer';
            
            ws.onmessage = async (event) => {
                if (event.data instanceof ArrayBuffer) {
                    const blob = new Blob([event.data], {type: 'image/jpeg'});
                    const img = await createImageBitmap(blob);
                    
                    ctx.drawImage(img, 0, 0, videoCanvas.width, videoCanvas.height);
                    frameCount++;
                    
                    const now = performance.now();
                    if (now - lastTime >= 1000) {
                        document.getElementById('fps').textContent = frameCount.toFixed(1);
                        frameCount = 0;
                        lastTime = now;
                    }
                } else {
                    const data = JSON.parse(event.data);
                    updateDetections(data.detections);
                }
            };
            
            ws.onclose = () => {
                setTimeout(connectWebSocket, 1000);
            };
        }
        
        function updateDetections(detections) {
            overlayCtx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
            document.getElementById('object-count').textContent = detections.length;
            
            const detectionList = document.getElementById('detection-list');
            detectionList.innerHTML = '';
            
            detections.forEach(det => {
                // 绘制检测框
                overlayCtx.strokeStyle = 'red';
                overlayCtx.lineWidth = 2;
                overlayCtx.strokeRect(det.x, det.y, det.width, det.height);
                
                // 绘制标签
                overlayCtx.fillStyle = 'rgba(255,0,0,0.7)';
                overlayCtx.fillRect(det.x, det.y - 20, det.label.length * 8 + 20, 20);
                overlayCtx.fillStyle = 'white';
                overlayCtx.fillText(
                    `${det.label} ${(det.confidence * 100).toFixed(0)}%`,
                    det.x + 5, det.y - 5
                );
                
                // 更新检测列表
                const div = document.createElement('div');
                div.textContent = `${det.label}: ${(det.confidence * 100).toFixed(0)}%`;
                detectionList.appendChild(div);
            });
        }
        
        // 初始化
        function init() {
            videoCanvas.width = 640;
            videoCanvas.height = 480;
            overlayCanvas.width = 640;
            overlayCanvas.height = 480;
            
            connectWebSocket();
            
            // 事件监听器
            document.getElementById('enable-detection').onchange = (e) => {
                ws.send(JSON.stringify({
                    command: 'setDetection',
                    enabled: e.target.checked
                }));
            };
            
            document.getElementById('confidence').oninput = (e) => {
                const value = e.target.value / 100;
                document.getElementById('confidence-value').textContent = value.toFixed(2);
                ws.send(JSON.stringify({
                    command: 'setConfidence',
                    threshold: value
                }));
            };
            
            document.getElementById('snapshot').onclick = () => {
                const canvas = document.createElement('canvas');
                canvas.width = videoCanvas.width;
                canvas.height = videoCanvas.height;
                const ctx = canvas.getContext('2d');
                ctx.drawImage(videoCanvas, 0, 0);
                ctx.drawImage(overlayCanvas, 0, 0);
                
                const link = document.createElement('a');
                link.download = 'snapshot.png';
                link.href = canvas.toDataURL();
                link.click();
            };
            
            document.getElementById('record').onclick = (e) => {
                if (!recording) {
                    startRecording();
                    e.target.textContent = 'Stop Recording';
                } else {
                    stopRecording();
                    e.target.textContent = 'Start Recording';
                }
                recording = !recording;
            };
        }
        
        function startRecording() {
            const stream = videoCanvas.captureStream();
            recordedChunks = [];
            mediaRecorder = new MediaRecorder(stream, {mimeType: 'video/webm'});
            
            mediaRecorder.ondataavailable = (e) => {
                if (e.data.size > 0) {
                    recordedChunks.push(e.data);
                }
            };
            
            mediaRecorder.onstop = () => {
                const blob = new Blob(recordedChunks, {type: 'video/webm'});
                const url = URL.createObjectURL(blob);
                const link = document.createElement('a');
                link.href = url;
                link.download = 'recording.webm';
                link.click();
            };
            
            mediaRecorder.start();
        }
        
        function stopRecording() {
            mediaRecorder.stop();
        }
        
        init();
    </script>
</body>
</html>
            )";
        } else {
            response.setStatusAndReason(HTTPResponse::HTTP_NOT_FOUND);
            response.send();
        }
    }
}

void WebServer::WebSocketHandler::handleWebSocket(WebSocket& ws) {
    try {
        ws.setBlocking(false);
        ws.setReceiveTimeout(Poco::Timespan(0, 100000)); // 100ms

        char buffer[1024];
        int flags;
        int n;
        
        while (true) {
            try {
                n = ws.receiveFrame(buffer, sizeof(buffer), flags);
                if (n > 0) {
                    // 处理客户端命令
                    std::string command(buffer, n);
                    // TODO: 处理命令
                }
            } catch (const Poco::TimeoutException&) {
                // 超时是正常的
            }

            // 获取并发送新帧
            auto frame = video_capture_->getLatestFrame();
            if (!frame.empty()) {
                try {
                    ws.sendFrame(frame.data(), frame.size(), WebSocket::FRAME_BINARY);
                    
                    // 处理图像识别
                    cv::Mat img = cv::imdecode(
                        cv::Mat(1, frame.size(), CV_8UC1, (void*)frame.data()),
                        cv::IMREAD_COLOR
                    );
                    
                    if (!img.empty()) {
                        auto detections = processor_->processFrame(img);
                        
                        // 发送检测结果
                        Poco::JSON::Object json;
                        json.set("type", "detections");
                        Poco::JSON::Array dets;
                        
                        for (const auto& det : detections) {
                            Poco::JSON::Object d;
                            d.set("label", det.label);
                            d.set("confidence", det.confidence);
                            d.set("x", det.bbox.x);
                            d.set("y", det.bbox.y);
                            d.set("width", det.bbox.width);
                            d.set("height", det.bbox.height);
                            dets.add(d);
                        }
                        
                        json.set("detections", dets);
                        std::stringstream ss;
                        json.stringify(ss);
                        ws.sendFrame(ss.str().data(), ss.str().size(), WebSocket::FRAME_TEXT);
                    }
                } catch (const std::exception&) {
                    break; // 连接可能已关闭
                }
            }
            
            std::this_thread::sleep_for(30ms);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "WebSocket handler error: " << e.what() << std::endl;
    }
}

WebServer::HandlerFactory::HandlerFactory(
    std::shared_ptr<CaptureInterface> capture,
    std::shared_ptr<ImageProcessor> processor)
    : video_capture_(capture), processor_(processor) {}

HTTPRequestHandler* WebServer::HandlerFactory::createRequestHandler(
    const HTTPServerRequest&) {
    return new WebSocketHandler(video_capture_, processor_);
}

WebServer::WebServer(std::shared_ptr<CaptureInterface> video_capture)
    : video_capture_(video_capture)
    , processor_(std::make_shared<ImageProcessor>()) {}

WebServer::~WebServer() {
    stop();
}

void WebServer::start(int port) {
    try {
        auto* params = new HTTPServerParams;
        params->setMaxQueued(100);
        params->setMaxThreads(4);

        ServerSocket socket(port);
        server_ = std::make_unique<HTTPServer>(
            new HandlerFactory(video_capture_, processor_), socket, params);
        server_->start();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        throw;
    }
}

void WebServer::stop() {
    if (server_) {
        server_->stop();
        server_.reset();
    }
} 