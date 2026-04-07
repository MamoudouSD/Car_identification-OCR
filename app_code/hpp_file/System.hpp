#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "Ai.hpp"
#include "Yolo_infer.hpp"
#include "Camera.hpp"
#include <semaphore>

/*
 * Summary:
 * Initialize system resources, hardware and shared structures.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - bool: true if initialization succeeds, false otherwise.
 */
bool system_init();

/*
 * Summary:
 * Capture frames from camera and push them to processing pipeline.
 *
 * Parameters:
 * - cam (Camera&): reference to camera object used for capture.
 * - semaphore: the semaphore for synchronization
 *
 * Returns:
 * - No return value.
 */
void capture(Camera& cam, std::binary_semaphore& capture_semaphore);

/*
 * Summary:
 * Run AI inference on captured frames.
 *
 * Parameters:
 * - model (Ai<std::vector<Detection>>*): pointer to AI inference model.
 *
 * Returns:
 * - No return value.
 */
void inference(Ai<std::vector<Detection>>* model);

/*
 * Summary:
 * Display processed frames on screen.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
void screen();

/*
 * Summary:
 * Save processed images and metadata to disk.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
void save();

/*
 * Summary:
 * Control execution timing and trigger system tasks.
 *
 * Parameters:
 * - id (int): sequencer identifier or signal value.
 *
 * Returns:
 * - No return value.
 */
void sequencer(int id);

/*
 * Summary:
 * Start system threads and execution workflow.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - bool: true if system starts successfully.
 */
bool start();

/*
 * Summary:
 * Handle system signal interruption.
 *
 * Parameters:
 * - id (int): received signal identifier.
 *
 * Returns:
 * - No return value.
 */
void signalHandler(int id);

/*
 * Summary:
 * Stop system and release resources.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - bool: true if shutdown succeeds.
 */
int system_end();

#endif