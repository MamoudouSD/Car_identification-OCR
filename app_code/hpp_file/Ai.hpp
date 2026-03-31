#ifndef AI_HPP
#define AI_HPP
#include <string>
#include "Notification.hpp"
#include <opencv2/opencv.hpp>


template <typename T>
class Ai{
    public:
        Ai(std::string path, Notification* n);
        virtual bool load_model() = 0;
        virtual T ai_inference(cv::Mat& img) = 0;
        virtual ~Ai();

    protected:
        std::string model_path;
        Notification* notif;
};

#endif

