#include "Image.hpp"

//annexe path : ./annexe
//folder_name: Weekday_M_d_y exp Sat_Jan_10_2026
Image::Image( Notification *n, int number){
    notif = n;
    nb = number;
    create_folder(annexe);
    time_t timestamp;
    time(&timestamp);
    folder_name = ctime(&timestamp);
    folder_name = string_clean(folder_name);
    folder_name = folder_name.substr(0, 10)+ "_" + folder_name.substr(20, folder_name.size());
    if (create_folder(annexe+"/"+folder_name)){
        write_file(annexe+"/"+folder_name+"/"+folder_name+".txt", "id;number_of_plate;plate_coordinates\n");
    }
}

bool Image::create_folder(std::string p){
    if (std::filesystem::exists(std::filesystem::path (p))){
        notif->notice_info(p + " folder already exist");
        return false;
    }else{
        create_directory( std::filesystem::path(p));
        notif->notice_info(p + " folder created");
        return true;
    }
}


bool Image::set_frame(cv::Mat img, const int camera_id){
    if (img.empty()) {
        notif->notice_err("Image color convertion ERROR: img is empty");
        return false;
    }else{
        cv::cvtColor(img, image_frame, cv::COLOR_BGR2RGB);
        cam_id= camera_id;
        set_imageName(camera_id);
    }
    return true;
}
void Image::set_imageName(const int camera_id){
    time_t timestamp;
    time(&timestamp);
    image_id = ctime(&timestamp);
    image_id = string_clean(image_id);
    image_id = std::to_string(camera_id) + "_" + image_id + "_" + std::to_string(nb);
}

//changera en fonction de ce que retournera AI
void Image::set_plateCoord(cv::Rect coord){
    plate_coord.push_back(coord);
}

//changera en fonction de ce que retournera AI
void Image::set_plateCoordScore(float score){
    plate_coord_score.push_back(score);
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
            cv::resize(plate, plate_resize,cv::Size(240, 320), 100.0, 100.0, cv::INTER_LANCZOS4);
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
        message += "\n";
    }
    write_file(annexe+"/"+folder_name+"/"+folder_name+".txt", message);
}

const cv::Mat Image::get_frame(){
    return image_frame;
}
const std::string Image::get_idImage(){
    return image_id;
}

const int Image::get_camId(){
    return cam_id;
}

std::vector<cv::Mat> Image::get_plateFrame(){
    return plate_frame;
}



namespace {
    std::string string_clean(std::string& s) {
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