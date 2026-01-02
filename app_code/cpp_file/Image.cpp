#include "Image.hpp"

void Image::convert_color(cv::Mat img){
    if (img.empty()) {
        notif->notice_err("Image color convertion ERROR: img is empty");
    }else{
        cv::cvtColor(img, colored_frame, cv::COLOR_RGBA2RGB);
    }
}
void Image::timestamp(const int camera_id){
    time_t timestamp;
    time(&timestamp);
    image_id = ctime(&timestamp);
    while(image_id.find(" ") < image_id.length()){
        image_id[image_id.find(" ")] = '_';
    }
    while(image_id.find(":") < image_id.length()){
        image_id[image_id.find(":")] = '-';
    }
    image_id = std::to_string(camera_id) + "_" + image_id;
}

void Image::set_plateCoord(std::vector <cv::Rect> coord){
    cv::Scalar color = (255, 0, 0);
    plate_coord = coord;
    for (const auto& c : coord){
        cv::rectangle(colored_frame, c, color, 2);
    }
}

void Image::reframe(cv::Mat img){
    if (img.empty()) {
        notif->notice_err("Image reframe ERROR: img is empty");
    }else{
        notif->notice_info("Image reframe info: rewrite");
    }
}
cv::Mat Image::get_frame(){
    return colored_frame;
}
std::string Image::get_id(){
    return image_id;
}
