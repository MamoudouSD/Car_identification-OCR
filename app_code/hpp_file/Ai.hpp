#ifndef AI_HPP
#define AI_HPP
#include <string>
#include "Notification.hpp"
#include <opencv2/opencv.hpp>

/*
 * Summary:
 * Generic abstract AI base class for model loading and inference.
 *
 * Parameters:
 * - T (template type): return type of the inference output.
 *
 * Returns:
 * - Not applicable (class definition).
 */
template <typename T>
class Ai{
    public:
    
        /*
         * Summary:
         * Initialize AI base class with model path and notification handler.
         *
         * Parameters:
         * - path (std::string): path to AI model file.
         * - n (Notification*): pointer to notification handler.
         *
         * Returns:
         * - No return value.
         */
        Ai(std::string path, Notification* n): model_path(path), notif(n) {}

        /*
         * Summary:
         * Load AI model into memory.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - bool: true if model loaded successfully, false otherwise.
         */
        virtual bool load_model() = 0;

        /*
         * Summary:
         * Run AI inference on input image.
         *
         * Parameters:
         * - img (cv::Mat&): input image.
         *
         * Returns:
         * - T: inference output defined by template type.
         */
        virtual T ai_inference(cv::Mat& img) = 0;

        /*
         * Summary:
         * Virtual destructor for safe inheritance.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - No return value.
         */
        virtual ~Ai()=default;

    protected:
        // path to model file
        std::string model_path;

        // notification handler
        Notification* notif = nullptr;
};

#endif

