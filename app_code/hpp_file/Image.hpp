#ifndef IMAGE_HPP
#define IMAGE_HPP
#endif

#include <opencv2/opencv.hpp>
#include <string>
#include <ctime>
#include <vector>
#include "Notification.hpp"
#include <filesystem>
#include <fstream>


class Image{
    public:
        Image( Notification *n);
        void set_frame(cv::Mat img, const int camera_id);
        void set_imageName(const int camera_id);
        void reframe();
        void save_frame();
        void save_plateInfo();
        void set_plateCoord(std::vector <cv::Rect> coord);
        void set_plateOcr(std::string ocr);
        cv::Mat get_frame();
        std::string get_id();


    private:
        void create_folder(std::string p);
        cv::Mat image_frame;
        std::vector <cv::Mat> plate_frame;
        std::vector <cv::Rect> plate_coord;
        std::vector <std::string> plate_ocr;
        std::string image_id;
        std::string folder_name;
        Notification* notif;
        const std::string annexe = "annexe";
        int nb;
};