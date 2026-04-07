#include "Camera.hpp"
#include "Notification.hpp"
#include <string>
#include <opencv2/opencv.hpp>


/*
 * Summary:
 * Initialize the Camera object with a notification handler.
 *
 * Parameters:
 * - n (Notification*): Pointer to notification object used for logging messages.
 *
 * Returns:
 * - No return value.
 */
Camera::Camera(Notification *n){
    notif = n;
}

/*
 * Summary:
 * Set the camera device identifier.
 *
 * Parameters:
 * - id (int): Camera device ID to assign.
 *
 * Returns:
 * - No return value.
 */
void Camera::set_idDevice(int id){
    id_device = id;
}

/*
 * Summary:
 * Initialize and configure the camera device.
 *
 * Parameters:
 * - id (int): Camera device ID to open.
 *
 * Returns:
 * - bool: True if camera opened successfully, false otherwise.
 */
bool Camera::cam_init(int id){
    set_idDevice(id);
    bool result = camera.open(id_device, api_preference, params);
    if (result == false){
        notif->notice_err("Error opening camera with id: " + std::to_string(id_device));
        return false;
    } else {
        camera.set(cv::CAP_PROP_BUFFERSIZE, 1);
        camera.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

       notif->notice_info("Camera with id: " + std::to_string(id_device) + " opened successfully");
        return true;
    }
}

/*
 * Summary:
 * Capture one frame from the opened camera.
 *
 * Parameters:
 * - No input parameters.
 *
 * Returns:
 * - bool: True if frame captured successfully, false otherwise.
 */
bool Camera::capture_frame(){
    if (!camera.isOpened()){
        notif->notice_err("Camera with id: " + std::to_string(id_device) + " is not opened");
        return false;
    }else {
        camera.read(frame);
        if (frame.empty()){
            notif->notice_err("Captured empty frame from camera with id: " + std::to_string(id_device));
            return false;
        }
    }
    return true;
}

/*
 * Summary:
 * Return the last captured frame.
 *
 * Parameters:
 * - No input parameters.
 *
 * Returns:
 * - cv::Mat: Last captured frame.
 */
cv::Mat Camera::get_frame(){
    if (frame.empty()){
         notif->notice_err("Captured empty frame from camera with id: " + std::to_string(id_device));
    }
    return frame;
}

/*
 * Summary:
 * Return camera device identifier.
 *
 * Parameters:
 * - No input parameters.
 *
 * Returns:
 * - int: Camera device ID.
 */
int Camera::get_idDevice(){
    return id_device;
}

/*
 * Summary:
 * Release camera resource when object is destroyed.
 *
 * Parameters:
 * - No input parameters.
 *
 * Returns:
 * - No return value.
 */
Camera::~Camera(){
    camera.release();
}