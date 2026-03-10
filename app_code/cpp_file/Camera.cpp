#include "Camera.hpp"

Camera::Camera(Notification *n){
    notif = n;
}

void Camera::set_idDevice(int id){
    id_device = id;
}

bool Camera::cam_init(int id){
    set_idDevice(id);
    bool result = camera.open(id_device, api_preference, params);
    if (result == false){
        notif->notice_err("Error opening camera with id: " + std::to_string(id_device));
        return false;
    } else {
        notif->notice_info("Camera with id: " + std::to_string(id_device) + " opened successfully");
        return true;
    }
}

void Camera::captureFrame(){
    if (!camera.isOpened()){
        notif->notice_err("Camera with id: " + std::to_string(id_device) + " is not opened");
    }else {
        camera.read(frame);
        if (frame.empty()){
            notif->notice_err("Captured empty frame from camera with id: " + std::to_string(id_device));
        }
    }
}

cv::Mat Camera::get_frame(){
    if (frame.empty()){
         notif->notice_err("Captured empty frame from camera with id: " + std::to_string(id_device));
    }
    return frame;
}

int Camera::get_idDevice(){
    return id_device;
}

Camera::~Camera(){
    camera.release();
    delete notif;
}