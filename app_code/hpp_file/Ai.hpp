#ifndef AI_HPP
#define AI_HPP
#endif

#include <executorch/extension/module/module.h>
#include <executorch/extension/tensor/tensor.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <vector>
#include <string>
#include "Notification.hpp"

using namespace ::executorch::extension;
template <typename T>

class Ai{
    public:
        Ai(std::string path, Notification* n);
        virtual int load_model() = 0;
        virtual TensorPtr data_processing() = 0;
        virtual T ai_inference() = 0;
        virtual ~Ai();

    protected:
        std::string model_path;
        Notification* notif;


};