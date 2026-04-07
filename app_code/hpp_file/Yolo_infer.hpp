#ifndef YOLO_INFER_HPP
#define YOLO_INFER_HPP

#include "Ai.hpp"
#include <opencv2/opencv.hpp>
#include <executorch/extension/module/module.h>
#include <executorch/extension/tensor/tensor.h>
#include <string>
#include <vector>

/*
 * Summary:
 * Detection structure used to store one predicted bounding box and its score.
 *
 * Parameters:
 * - None (structure definition)
 *
 * Returns:
 * - Not applicable
 */
struct Detection{
    // confidence score of detection
    float scores;

    // bounding box coordinates
    cv::Rect boxes;
};

/*
 * Summary:
 * YOLO inference class used to load model, preprocess images and return detections.
 *
 * Parameters:
 * - None (class definition)
 *
 * Returns:
 * - Not applicable
 */
class Yolo_infer:public Ai<std::vector<Detection>>{
    public:

        /*
         * Summary:
         * Initialize YOLO inference object with model path and thresholds.
         *
         * Parameters:
         * - path (std::string): model file path.
         * - n (Notification*): pointer to notification handler.
         * - scoreThreshold (float): confidence threshold for detections.
         * - nmsThreshold (float): NMS threshold for box filtering.
         *
         * Returns:
         * - No return value.
         */
        Yolo_infer(std::string path, Notification* n, float scoreThreshold, float nmsThreshold);

        /*
         * Summary:
         * Load YOLO model into memory.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - bool: true if model is loaded successfully, false otherwise.
         */
        bool load_model();

        /*
         * Summary:
         * Convert image into tensor format for inference.
         *
         * Parameters:
         * - img (cv::Mat&): input image.
         * - CHW_frame (cv::Mat&): output blob/image tensor representation.
         * - unsqueeze (bool): optional flag for adding extra dimension.
         * - unsqueeze_dim (int): target dimension for unsqueeze operation.
         *
         * Returns:
         * - executorch::extension::TensorPtr: tensor ready for model inference.
         */
        executorch::extension::TensorPtr data_processing(cv::Mat& img, cv::Mat& CHW_frame, bool unsqueeze=false, int unsqueeze_dim = 0);

        /*
         * Summary:
         * Compute confidence score from logits value.
         *
         * Parameters:
         * - logits (float): raw output value from model.
         *
         * Returns:
         * - float: transformed confidence score.
         */
        float calcul_scores(float logits);

        /*
         * Summary:
         * Run YOLO inference on image and return detections.
         *
         * Parameters:
         * - image (cv::Mat&): input image for inference.
         *
         * Returns:
         * - std::vector<Detection>: list of detected objects.
         */
        std::vector<Detection> ai_inference(cv::Mat& image);

        /*
         * Summary:
         * Destroy YOLO inference object and release resources.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - No return value.
         */
        virtual ~Yolo_infer();
    
    private:
    
        // confidence threshold used to keep detections
        float modelScoreThreshold;

        // NMS threshold used to filter overlapping detections
        float modelNMSThreshold;

        // pointer to executorch module
        executorch::extension::Module* module = nullptr;
};
#endif
