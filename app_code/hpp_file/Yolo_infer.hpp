#ifndef YOLO_INFER_HPP
#define YOLO_INFER_HPP
#endif

#include "Ai.hpp"

struct Detection{
    float scores;
    cv::Rect boxes;
};

class Yolo_infer:public Ai{
    public:
        Yolo_infer(std::string path, Notification* n, float scoreThreshold, float nmsThreshold);
        int load_model();
        auto data_processing(cv::Mat img, cv::Mat CHW_frame, bool unsqueeze=false, int unsqueeze_dim = 0);
        float calcul_scores(float logits);
        std::vector<Detection> ai_inference(cv::Mat& image);
        virtual ~Yolo_infer();
    
    private:
        float modelScoreThreshold;
        float modelNMSThreshold;
        Module* module;
};