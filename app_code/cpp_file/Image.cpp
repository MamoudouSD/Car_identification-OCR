#include "Image.hpp"

//annexe path : ./annexe
//folder_name: Weekday_M_d_y exp Sat_Jan_10_2026
Image::Image( Notification *n){
    notif = n;
    create_folder(annexe);
    time_t timestamp;
    time(&timestamp);
    folder_name = ctime(&timestamp);
    folder_name = string_cleang(folder_name);
    folder_name = image_id.substr(0, 10)+ "_" + image_id.substr(20, image_id.size());
    create_folder(annexe+"/"+folder_name);
    write_file(annexe+"/"+folder_name+"/"+folder_name+".txt", "id;number_of_plate;plate_coordinates;plate_ocr\n");
}

void Image::create_folder(std::string p){
    if (std::filesystem::exists(std::filesystem::path (p))){
        notif->notice_info(p + " folder already exist");
    }else{
        create_directory( std::filesystem::path(p));
        notif->notice_info(p + " folder created");
    }
}

void Image::set_frame(cv::Mat img, const int camera_id){
    if (img.empty()) {
        notif->notice_err("Image color convertion ERROR: img is empty");
    }else{
        nb++;
        cv::cvtColor(img, image_frame, cv::COLOR_BGR2RGB);
        set_imageName(camera_id);
    }
}
void Image::set_imageName(const int camera_id){
    time_t timestamp;
    time(&timestamp);
    image_id = ctime(&timestamp);
    image_id = string_cleang(image_id);
    image_id = std::to_string(camera_id) + "_" + image_id + "_" + std::to_string(nb);
}

//changera en fonction de ce que retournera AI
void Image::set_plateCoord(std::vector <cv::Rect> coord){
    plate_coord = coord;
    reframe();
    
}

//changera en fonction de ce que retournera AI
void Image::set_plateCoordScore(std::vector <float> score){
    plate_coord_score = score;
}

void Image::save_frame(){
    cv::Scalar color = (0, 0, 255);
    for (const auto& c : plate_coord){
        cv::rectangle(image_frame, c, color, 2);
    }
    if (cv::imwrite(annexe+"/"+folder_name+"/"+image_id+".jpeg", image_frame)){
        notif->notice_info("Image saved");
        save_plateInfo();
    }else{
        notif->notice_err("Image not saved");
    }
}

void Image::reframe(){
    if (image_frame.empty()) {
        notif->notice_err("Image reframe ERROR: img is empty");
    }else{
        cv::Mat plate;
        cv::Mat plate_resize;
        for (int i =0; i<plate_coord.size(); i++){
            plate = image_frame(plate_coord[i]);
            cv::resize(plate, plate_resize,cv::Size(384, 384), 100.0, 100.0, cv::INTER_LANCZOS4);
            plate_frame.push_back (plate_resize);
        }
    }
}

void Image::save_plateInfo(){
    std::string message;
    if (plate_coord.size()==0){
        message = image_id+";0;0;Nan\n";
    }else{
        message = image_id+";"+ std::to_string(plate_coord.size())
                            +";("+std::to_string(plate_coord[0].x)+","
                            +std::to_string(plate_coord[0].y)+","+std::to_string(plate_coord[0].width)+","
                            +std::to_string(plate_coord[0].height)+")";
        
        for (int i =1; i< plate_coord.size(); i++){
            message +=",";
            message += "("+std::to_string(plate_coord[i].x)+","+std::to_string(plate_coord[i].y)+","+std::to_string(plate_coord[i].width)
                    +","+std::to_string(plate_coord[i].height)+")";
        }

        message += ";"+plate_ocr[0];
        for (int i=1; i<plate_ocr.size(); i++){
            message += ",";
            message += plate_ocr[i];
        }
        message += "\n";
    }
    write_file(annexe+"/"+folder_name+"/"+folder_name+".txt", message);
}

cv::Mat Image::get_frame(){
    return image_frame;
}
std::string Image::get_id(){
    return image_id;
}

void Image::set_plateOcr(std::string ocr){
    plate_ocr.push_back(ocr);
}

namespace {
    std::string string_cleang(std::string& s) {
        // Logique de découpe ou mise en forme
        while(s.find(" ") < s.length()){
            s[s.find(" ")] = '_';
        }
        while(s.find(":") < s.length()){
            s[s.find(":")] = '-';
        }
        return s;
    }

    void write_file(std::string name, std::string mess){
        std::ofstream MyFile(name, std::ios::app);
        // Write to the file
        MyFile << mess;
        // Close the file
        MyFile.close();
    }
}