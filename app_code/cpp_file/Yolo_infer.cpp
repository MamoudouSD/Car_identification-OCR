#include "Yolo_infer.hpp"

Yolo_infer::Yolo_infer(std::string path, Notification* n, float scoreThreshold, float nmsThreshold): Ai(path, notif){
    modelScoreThreshold = scoreThreshold;
    modelNMSThreshold = nmsThreshold;
}

int Yolo_infer::load_model(){
    module = new Module(model_path);
    auto r = module->load();
    if (r != executorch::runtime::Error::Ok) {
        notif->notice_err("Yolo loading failed: model load failed!");
        return 1;
    }else{
        notif->notice_err("Yolo loading succed: model load!");
        return 0
    }
}

auto Yolo_infer::data_processing(cv::Mat img, cv::Mat CHW_frame, bool unsqueeze=false, int unsqueeze_dim = 0){
    cv::Mat f32_img;
    img.convertTo(f32_img, CV_32FC3);
    CHW_frame = cv::dnn::blobFromImage(f32_img, (1.0/255.0), f32_img.size(), cv::Scalar(0, 0, 0), false, false, CV_32F);
    TensorPtr tensor_image = from_blob(CHW_frame.ptr<float>(), {1, 3, CHW_frame.size[2], CHW_frame.size[3]}, executorch::aten::ScalarType::Float);
    return tensor_image;
}

float Yolo_infer::calcul_scores(float logits){
    float score = 1.0/(1.0+std::exp(-logits));
    return score;
}

std::vector<Detection> Yolo_infer::ai_inference(cv::Mat& image){
    cv::Mat CHW_frame;
    std::vector <Detection> detections;
    auto img_tensor = data_processing(img, CHW_frame);
    int result_load = load_model();
    if (result_load){
        const auto result_forward = module->forward(img_tensor);
        if (result_forward.ok()) {
            const auto output = result.get();
            executorch::aten::Tensor row_score = output[2].toTensor();
            executorch::aten::Tensor coor = output[0].toTensor();
            const float* row_score_data = row_score.const_data_ptr<float>();
            const float* coor_data = coor.const_data_ptr<float>();

            for (int i =0 ; i<8400; i++){
                float sc = calcul_scores (*(row_score_data+i)); 
                if (sc >= modelScoreThreshold){
                    float x = *(coor_data+(i));
                    float y = *(coor_data+(8400+i));
                    float w = *(coor_data+(16800+i));
                    float h = *(coor_data+(25200+i));
                    int left = int(x - 0.5f * w);
                    int top = int(y - 0.5f * h);
                    plate_coord.push_back(cv::Rect (left, top, w, h));
                    scores.push_back(sc);
                }
            }

            std::vector<int> nms_result;
            cv::dnn::NMSBoxes(plate_coord, scores, modelScoreThreshold, modelNMSThreshold, nms_result);

            for (int i = 0; i<nms_result.size(); i++){
                Detection result;
                int idx = nms_result[i];
                result.boxes=plate_coord[idx];
                result.scores = scores[idx];
                detections.push_back(result);
            }
            notif->notice_info("Yolo Inference done");
        }else{
            notif->notice_err("Yolo Inference failed: forward failed!");
        }
    }
    return detections;
}

Yolo_infer::~Yolo_infer(){
    delete module
}