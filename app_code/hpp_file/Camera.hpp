#ifndef CAMERA_HPP
#define CAMERA_HPP
#endif

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "Notification.hpp"

class Camera {
    public:
        Camera(int id, Notification *n);
        void captureFrame();
        cv::Mat get_frame();
        int get_idDevice();
        virtual ~Camera();

    private:
        std::vector< int > params = {
            cv::CAP_PROP_FRAME_WIDTH, 640,
            cv::CAP_PROP_FRAME_HEIGHT, 480,
            cv::CAP_PROP_FPS, 30};
        Notification *notif;
        int id_device;
        cv::VideoCapture camera;
        cv::Mat frame;
        int api_preference = cv::CAP_ANY;
};