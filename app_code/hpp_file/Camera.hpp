#ifndef CAMERA_HPP
#define CAMERA_HPP
#endif

#include "Notification.hpp"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>


class Camera {
    public:
        Camera(Notification *n);
        bool cam_init(int id);
        void set_idDevice(int id);
        bool captureFrame();
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
