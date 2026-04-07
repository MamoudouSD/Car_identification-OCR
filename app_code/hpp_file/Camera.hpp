#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "Notification.hpp"
#include <opencv2/opencv.hpp>
#include <vector>

/*
 * Summary:
 * Camera class used to initialize, capture and retrieve frames from a video device.
 *
 * Parameters:
 * - None (class definition)
 *
 * Returns:
 * - Not applicable
 */
class Camera {
    public:

        /*
         * Summary:
         * Initialize camera object with notification handler.
         *
         * Parameters:
         * - n (Notification*): pointer to notification object.
         *
         * Returns:
         * - No return value.
         */
        Camera(Notification *n);

        /*
         * Summary:
         * Initialize camera using device ID.
         *
         * Parameters:
         * - id (int): camera device identifier.
         *
         * Returns:
         * - bool: true if camera initialized successfully.
         */
        bool cam_init(int id);

        /*
         * Summary:
         * Set camera device identifier.
         *
         * Parameters:
         * - id (int): camera device identifier.
         *
         * Returns:
         * - No return value.
         */
        void set_idDevice(int id);

        /*
         * Summary:
         * Capture a frame from the camera.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - bool: true if frame captured successfully.
         */
        bool capture_frame();

        /*
         * Summary:
         * Return last captured frame.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - cv::Mat: captured image frame.
         */
        cv::Mat get_frame();

        /*
         * Summary:
         * Return camera device identifier.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - int: camera device ID.
         */
        int get_idDevice();

        /*
         * Summary:
         * Release camera resources.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - No return value.
         */
        virtual ~Camera();

        
    private:

        // camera configuration parameters (width, height, fps)
        std::vector< int > params = {
            cv::CAP_PROP_FRAME_WIDTH, 640,
            cv::CAP_PROP_FRAME_HEIGHT, 480,
            cv::CAP_PROP_FPS, 30};

        // notification handler
        Notification *notif = nullptr;

        // camera device ID
        int id_device;

        // OpenCV video capture object
        cv::VideoCapture camera;

        // last captured frame
        cv::Mat frame;
        
        // API preference for camera backend
        int api_preference = cv::CAP_ANY;
};
#endif

