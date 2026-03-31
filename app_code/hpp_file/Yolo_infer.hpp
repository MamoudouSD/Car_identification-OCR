#ifndef YOLO_INFER_HPP
#define YOLO_INFER_HPP
#include "Ai.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <executorch/extension/module/module.h>
#include <executorch/extension/tensor/tensor.h>
#include <vector>

struct Detection{
    float scores;
    cv::Rect boxes;
};

class Yolo_infer:public Ai<std::vector<Detection>>{
    public:
        Yolo_infer(std::string path, Notification* n, float scoreThreshold, float nmsThreshold);
        bool load_model();
        executorch::extension::TensorPtr data_processing(cv::Mat& img, cv::Mat& CHW_frame, bool unsqueeze=false, int unsqueeze_dim = 0);
        float calcul_scores(float logits);
        std::vector<Detection> ai_inference(cv::Mat& image);
        virtual ~Yolo_infer();
    
    private:
        float modelScoreThreshold;
        float modelNMSThreshold;
        executorch::extension::Module* module = nullptr;
};
#endif
