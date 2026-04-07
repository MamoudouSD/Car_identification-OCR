
//commencer par cam2 ()

#include "System.hpp"
#include "ThreadSafe_queue.hpp"
#include "Yolo_infer.hpp"
#include "Image.hpp"
#include "Notification.hpp"
#include "Display.hpp"
#include "Camera.hpp"
#include "Ai.hpp"
#include <pthread.h>
#include <atomic>
#include <vector>
#include <chrono>
#include <thread>
#include <signal.h>
#include <time.h>
#include <semaphore>

// global notification handler
Notification notif("Plate_detection");

// camera objects
Camera cam1(&notif);
Camera cam2(&notif);

// AI model
Ai<std::vector<Detection>>* yolo = new Yolo_infer("../Ai_file/model_quantified_executorch.pte", &notif, 0.25, 0.6);

// thread-safe queues
ThreadSafe_queue<Image> img_toAi(30);
ThreadSafe_queue<Image> img_toDisplay(30);
ThreadSafe_queue<Image> img_toSave(50);

// thread control flags
std::atomic<bool> capture_v (true);
std::atomic<bool> inference_v (true);
std::atomic<bool> screen_v (true);
std::atomic<bool> save_v (true);
std::atomic<bool> sequencer_v (true);

// synchronization semaphores
std::binary_semaphore capture1_semaphore (0);
std::binary_semaphore capture2_semaphore (0);
std::binary_semaphore inference_semaphore (0);
std::binary_semaphore screen_semaphore (0);
std::binary_semaphore save_semaphore (0);

// image, sequencer counter
int nb =0;
int seqCnt = 0;

// realtime timer
timer_t timer_1;
struct itimerspec itime;

/*
 * Summary:
 * Initialize system components (display, cameras, AI model).
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - bool: true if initialization successful, false otherwise.
 */
bool system_init(){
    bool result;
    uint16_t col1=GREEN;
    uint16_t col2=GREEN;
    gpio_init();
    display_init();

    result = cam1.cam_init(2);
    if (!result){
        col1 = RED;
    }
    result = cam2.cam_init(0);
    if (!result){
        col2 = RED;
    }
    fill_screen(col1, col2);
    if (col1 == RED or col2 == RED){
        return false;
    }else{
        result = yolo->load_model();
        if (!result){
            return false;
        }
    }
    return true;
}

/*
 * Summary:
 * Capture images from camera and push them to AI queue.
 *
 * Parameters:
 * - cam (Camera&): camera object
 * - capture_semaphore (std::binary_semaphore&): capture synchronization semaphore
 *
 * Returns:
 * - No return value.
 */
void capture(Camera& cam, std::binary_semaphore& capture_semaphore){
    while (capture_v){
        capture_semaphore.acquire();
        if (cam.capture_frame()){
            Image img(&notif, nb);
            if(img.set_frame(cam.get_frame().clone(), cam.get_idDevice())){
                nb++;
                img_toAi.push(img);
            }
        }
    }
}

/*
 * Summary:
 * Perform AI inference on captured images.
 *
 * Parameters:
 * - model (Ai<std::vector<Detection>>*): AI detection model
 *
 * Returns:
 * - No return value.
 */
void inference(Ai<std::vector<Detection>>* model){
    while(inference_v){
        inference_semaphore.acquire();
        if(!img_toAi.empty()){
            auto img = img_toAi.pop();
            auto result = model->ai_inference(img.get_frame());
            if (!result.empty()){
                for (int i =0; i<result.size(); i++){
                    img.set_plateCoord(result[i].boxes);
                    img.set_plateCoordScore(result[i].scores);
                }
                img.reframe();
                img_toDisplay.push(img);
                img_toSave.push(img);
            }else{
                std::string msg = "No plate detection for img "+ img.get_idImage();
                notif.notice_info(msg);
                img_toSave.push(img);
            }
        }
    }
    
}

/*
 * Summary:
 * Display detected plate images on screen.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
void screen(){
    while (screen_v){
        screen_semaphore.acquire();
        if (!img_toDisplay.empty()){
            auto img = img_toDisplay.pop();
            auto plate = img.get_plateFrame();
            if (!plate.empty()){
                fill_image(plate[0], img.get_camId());
            }
        }
    }
}

/*
 * Summary:
 * Save images and plate information.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
void save(){
    while (save_v){
        save_semaphore.acquire();
        if (!img_toSave.empty()){
            auto img = img_toSave.pop();
            img.save_frame();
            img.save_plateInfo();
        }
    }
}

/*
 * Summary:
 * Sequencer controlling periodic task execution.
 *
 * Parameters:
 * - id (int): signal identifier
 *
 * Returns:
 * - No return value.
 */
void sequencer(int id){
    seqCnt++;

    if (seqCnt % 48 == 0) {
        capture1_semaphore.release();
        capture2_semaphore.release();
    }

    if (seqCnt % 35 == 0) {
        save_semaphore.release();
        screen_semaphore.release();
    }

    if (seqCnt % 70 == 0) {
        inference_semaphore.release();
    }

    if (!sequencer_v){
        itime.it_interval.tv_sec = 0;
        itime.it_interval.tv_nsec = 0;
        itime.it_value.tv_sec = 0;
        itime.it_value.tv_nsec = 0;

        timer_settime(timer_1, 0, &itime, NULL);

        capture_v = false;
        inference_v = false;
        screen_v = false;
        save_v = false;

        capture1_semaphore.release();
        capture2_semaphore.release();
        inference_semaphore.release();
        save_semaphore.release();
        screen_semaphore.release();
    }
    
}

/*
 * Summary:
 * Handle SIGINT signal to stop system.
 *
 * Parameters:
 * - id (int): signal identifier
 *
 * Returns:
 * - No return value.
 */
void signalHandler(int id){
    sequencer_v = false;
}

/*
 * Summary:
 * Set real-time priority and CPU affinity for thread.
 *
 * Parameters:
 * - pr (int): priority level
 * - cpu (int): CPU core id
 *
 * Returns:
 * - No return value.
 */
void set_realtime_priority(int pr, int cpu) {
    struct sched_param param;
    param.sched_priority = pr;

    if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0) {
        notif.notice_err("Erreur : Impossible de definir les parametres de priorite");
        exit(-1);
    } else {
        notif.notice_info("Priorite en temps reel definie avec succes");
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        notif.notice_err("Erreur lors de la définition de l'affinité CPU");
        exit(-1);
    } else {
        notif.notice_info("Affinité CPU définie avec succès sur CPU");
    }
}

/*
 * Summary:
 * Start all system threads and realtime scheduler.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - bool: true when execution finishes.
 */
bool start(){
    
    set_realtime_priority(89, 3);

    signal(SIGINT, signalHandler);

    std::thread cam1_thread ([](){
        set_realtime_priority(80, 0);
        capture(cam1, capture1_semaphore);
    });

    std::thread cam2_thread ([](){
        set_realtime_priority(80, 1);
        capture(cam2, capture2_semaphore);
    });

    std::thread inference_thread ([](){
        set_realtime_priority(90, 2);
        inference(yolo);
    });

    std::thread screen_thread ([](){
        set_realtime_priority(60, 0);
        screen();
    });

    std::thread save_thread ([](){
        set_realtime_priority(60, 1);
        save();
    });

    timer_create(CLOCK_REALTIME, NULL, &timer_1);

    signal(SIGALRM, sequencer);

    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_nsec = 5000000;
    itime.it_value.tv_sec = 0;
    itime.it_value.tv_nsec = 5000000;

    timer_settime(timer_1, 0, &itime, NULL);

    cam1_thread.join();
    cam2_thread.join();
    inference_thread.join();
    screen_thread.join();
    save_thread.join();

    return true;
}


/*
 * Summary:
 * Flush remaining data and stop system.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - bool: true when cleanup finished.
 */
bool system_end(){
    while (!img_toAi.empty()){
        auto img = img_toAi.pop();
        auto result = yolo->ai_inference(img.get_frame());
        if (!result.empty()){
            for (int i =0; i<result.size(); i++){
                img.set_plateCoord(result[i].boxes);
                img.set_plateCoordScore(result[i].scores);
            }
            img.reframe();
            img_toDisplay.push(img);
            img_toSave.push(img);
        }else{
            std::string msg = "No plate detection for img "+ img.get_idImage();
            notif.notice_info(msg);
            img_toSave.push(img);
        }
    }

    while (!img_toSave.empty()){
        auto img = img_toSave.pop();
        img.save_frame();
        img.save_plateInfo();
    }
    notif.notice_info("Program end");
    return true;
}
