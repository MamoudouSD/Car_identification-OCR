#ifndef IMAGE_HPP
#define IMAGE_HPP
#endif

#include "Notification.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/*
 * Summary:
 * Image class used to store, process, crop and save captured images and plate data.
 *
 * Parameters:
 * - None (class definition)
 *
 * Returns:
 * - Not applicable
 */
class Image{
    public:

        /*
         * Summary:
         * Initialize Image object with notification handler and image number.
         *
         * Parameters:
         * - n (Notification*): pointer to notification object.
         * - number (int): image sequence number.
         *
         * Returns:
         * - No return value.
         */
        Image( Notification *n, int number);

        /*
         * Summary:
         * Set image frame and associated camera identifier.
         *
         * Parameters:
         * - img (cv::Mat): input image.
         * - camera_id (const int): identifier of source camera.
         *
         * Returns:
         * - bool: true if frame is set successfully.
         */
        bool set_frame(cv::Mat img, const int camera_id);

        /*
         * Summary:
         * Generate image identifier using camera ID and timestamp.
         *
         * Parameters:
         * - camera_id (const int): identifier of source camera.
         *
         * Returns:
         * - No return value.
         */
        void set_imageName(const int camera_id);

        /*
         * Summary:
         * Crop and resize plate regions from image frame.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - No return value.
         */
        void reframe();

        /*
         * Summary:
         * Save image frame with plate rectangles.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - No return value.
         */
        void save_frame();

        /*
         * Summary:
         * Save detected plate information to file.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - No return value.
         */
        void save_plateInfo();

        /*
         * Summary:
         * Store detected plate coordinates.
         *
         * Parameters:
         * - coord (cv::Rect): bounding box of detected plate.
         *
         * Returns:
         * - No return value.
         */
        void set_plateCoord(cv::Rect coord);

        /*
         * Summary:
         * Store detected plate confidence score.
         *
         * Parameters:
         * - score (float): confidence score of detected plate.
         *
         * Returns:
         * - No return value.
         */
        void set_plateCoordScore(float score);

        /*
         * Summary:
         * Return image frame reference.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - cv::Mat&: reference to stored image frame.
         */
        cv::Mat& get_frame();

        /*
         * Summary:
         * Return image identifier.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - std::string: image unique identifier.
         */
        const std::string get_idImage();

        /*
         * Summary:
         * Return source camera identifier.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - int: camera identifier.
         */
        const int get_camId();

        /*
         * Summary:
         * Return cropped plate images.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - std::vector<cv::Mat>: list of cropped plate images.
         */
        std::vector<cv::Mat> get_plateFrame();


    private:

        /*
         * Summary:
         * Create folder if it does not already exist.
         *
         * Parameters:
         * - p (std::string): folder path.
         *
         * Returns:
         * - bool: true if folder created, false otherwise.
         */
        bool create_folder(std::string p);
        
        // full image frame
        cv::Mat image_frame;

        // cropped plate images
        std::vector<cv::Mat> plate_frame;

        // detected plate coordinates
        std::vector<cv::Rect> plate_coord;

        // detected plate confidence scores
        std::vector<float> plate_coord_score;

        // image unique identifier
        std::string image_id;

        // folder name used for saving outputs
        std::string folder_name;

        // notification handler
        Notification* notif;

        // base output folder
        const std::string annexe = "../annexe";

        // image sequence number
        int nb;

        // source camera identifier
        int cam_id;
};