#include "Image.hpp"
#include "Notification.hpp"
#include <opencv2/opencv.hpp>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

/*
 * Summary:
 * Clean string by replacing spaces and ':' characters.
 *
 * Parameters:
 * - s (std::string&): input string to clean
 *
 * Returns:
 * - std::string: cleaned string
 */
    std::string string_clean(std::string& s) {
        while(s.find(" ") < s.length()){
            s[s.find(" ")] = '_';
        }
        while(s.find(":") < s.length()){
            s[s.find(":")] = '-';
        }
        return s.substr(0, s.size()-1);
    }

/*
 * Summary:
 * Append message to file.
 *
 * Parameters:
 * - name (std::string): file path
 * - mess (std::string): message to write
 *
 * Returns:
 * - No return value.
 */
    void write_file(std::string name, std::string mess){
        std::ofstream MyFile(name, std::ios::app);
        // Write to the file
        MyFile << mess;
        // Close the file
        MyFile.close();
    }
}

/*
 * Summary:
 * Initialize Image object and create storage folder.
 * folder_name form: Weekday_M_d_y
 *
 * Parameters:
 * - n (Notification*): notification handler
 * - number (int): image index number
 *
 * Returns:
 * - No return value.
 */
Image::Image( Notification *n, int number){
    notif = n;
    nb = number;
    create_folder(annexe);
    time_t timestamp;
    time(&timestamp);
    folder_name = ctime(&timestamp);
   folder_name = string_clean(folder_name);
    folder_name = folder_name.substr(0, 10)+ "_" + folder_name.substr(20, folder_name.size()-5);
    if (create_folder(annexe+"/"+folder_name)){
        write_file(annexe+"/"+folder_name+"/"+folder_name+".txt", "id;number_of_plate;plate_coordinates\n");
    }
}

/*
 * Summary:
 * Create directory if it does not exist.
 *
 * Parameters:
 * - p (std::string): folder path
 *
 * Returns:
 * - bool: true if folder created, false if already exists
 */
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

/*
 * Summary:
 * Set image frame and convert color format.
 *
 * Parameters:
 * - img (cv::Mat): input image
 * - camera_id (int): camera identifier
 *
 * Returns:
 * - bool: true if frame set successfully, false otherwise
 */
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

/*
 * Summary:
 * Generate image unique identifier.
 *
 * Parameters:
 * - camera_id (int): camera identifier
 *
 * Returns:
 * - No return value.
 */
void Image::set_imageName(const int camera_id){
    time_t timestamp;
    time(&timestamp);
    image_id = ctime(&timestamp);
    image_id = string_clean(image_id);
    image_id = std::to_string(camera_id) + "_" + image_id + "_" + std::to_string(nb);
}

/*
 * Summary:
 * Store plate coordinates.
 *
 * Parameters:
 * - coord (cv::Rect): bounding box coordinates
 *
 * Returns:
 * - No return value.
 */
void Image::set_plateCoord(cv::Rect coord){
    plate_coord.push_back(coord);
}

/*
 * Summary:
 * Store plate detection score.
 *
 * Parameters:
 * - score (float): detection confidence
 *
 * Returns:
 * - No return value.
 */
void Image::set_plateCoordScore(float score){
    plate_coord_score.push_back(score);
}

/*
 * Summary:
 * Draw detected plates and save image.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
void Image::save_frame(){
    cv::Scalar color = (0, 0, 255);
    for (const auto& c : plate_coord){
        cv::rectangle(image_frame, c, color, 2);
    }
    if (cv::imwrite(annexe+"/"+folder_name+"/"+image_id+".jpeg", image_frame)){
        notif->notice_info("Image saved");
    }else{
        notif->notice_err("Image not saved");
    }
}

/*
 * Summary:
 * Extract plate regions and resize them.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
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

/*
 * Summary:
 * Save plate coordinates information to file.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
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

/*
 * Summary:
 * Return image frame reference.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - cv::Mat&: reference to image frame
 */
cv::Mat& Image::get_frame(){
    return image_frame;
}

/*
 * Summary:
 * Return image identifier.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - std::string: image ID
 */
const std::string Image::get_idImage(){
    return image_id;
}

/*
 * Summary:
 * Return camera identifier.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - int: camera ID
 */
const int Image::get_camId(){
    return cam_id;
}

/*
 * Summary:
 * Return cropped plate frames.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - std::vector<cv::Mat>: list of plate images
 */
std::vector<cv::Mat> Image::get_plateFrame(){
    return plate_frame;
}