#include "gst_capture.h"

GstCapture::GstCapture() {
    gst_init(nullptr, nullptr);
}

GstCapture::~GstCapture() {
    stop();
}

bool GstCapture::start(int device_id) {
    char pipeline_str[256];
    snprintf(pipeline_str, sizeof(pipeline_str),
        "v4l2src device=/dev/video%d ! "
        "video/x-raw,width=640,height=480 ! "
        "videoconvert ! jpegenc ! "
        "appsink name=sink", device_id);
    
    pipeline_ = gst_parse_launch(pipeline_str, nullptr);
    if (!pipeline_) return false;
    
    appsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "sink");
    g_signal_connect(appsink_, "new-sample",
                    G_CALLBACK(newSampleCallback), this);
    
    running_ = true;
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    
    return true;
}

void GstCapture::stop() {
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
    
    if (appsink_) {
        gst_object_unref(appsink_);
        appsink_ = nullptr;
    }
    
    running_ = false;
}

void GstCapture::newSampleCallback(GstElement* sink, GstCapture* self) {
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if (!sample) return;
    
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        std::string frame_data(reinterpret_cast<char*>(map.data), map.size);
        {
            std::lock_guard<std::mutex> lock(self->frame_mutex_);
            self->latest_frame_ = std::move(frame_data);
        }
        gst_buffer_unmap(buffer, &map);
    }
    
    gst_sample_unref(sample);
}

std::string GstCapture::getLatestFrame() {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    return latest_frame_;
} 