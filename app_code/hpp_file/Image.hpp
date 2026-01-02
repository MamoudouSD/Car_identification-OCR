#ifndef IMAGE_HPP
#define IMAGE_HPP
#endif

#include <opencv2/opencv.hpp>
#include <string>
#include <ctime>
#include <vector>
#include "Notification.hpp"


class Image{
    public:
        void convert_color(cv::Mat img);
        void timestamp(const int camera_id);
        void reframe(cv::Mat img);
        void save_frame();
        void set_plateCoord(std::vector <cv::Rect> coord);
        cv::Mat get_frame();
        std::string get_id();


    private:
        cv::Mat colored_frame;
        std::vector <cv::Mat> reframed_frame;
        std::vector <cv::Rect> plate_coord;
        std::string image_id;
        Notification* notif;
};