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
        Image( Notification *n, int number);
        bool set_frame(cv::Mat img, const int camera_id);
        void set_imageName(const int camera_id);
        void reframe();
        void save_frame();
        void save_plateInfo();
        void set_plateCoord(cv::Rect coord);
        void set_plateCoordScore(float score);
        const cv::Mat get_frame();
        const std::string get_idImage();
        const int get_camId();
        std::vector<cv::Mat> get_plateFrame();


    private:
        bool create_folder(std::string p);
        cv::Mat image_frame;
        std::vector <cv::Mat> plate_frame;
        std::vector <cv::Rect> plate_coord;
        std::vector <float> plate_coord_score;
        std::string image_id;
        std::string folder_name;
        Notification* notif;
        const std::string annexe = "annexe";
        int nb;
        int cam_id;
};