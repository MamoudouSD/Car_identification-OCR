#include "System.hpp"
#include <pthread/pthread_impl.h>

Notification notif("Plate_detection");
Camera cam1(&notif);
Camera cam2(&notif);
Ai* yolo = new Yolo_infer("Ai_file/model_quantified_executorch.pte", &notif, 0.25, 0.6);
int nb =0;
ThreadSafe_queue<Image> img_toAi(30);
ThreadSafe_queue<Image> img_toDisplay(30);
ThreadSafe_queue<Image> img_toSave(50);

std::atomic<bool> capture_v (true);
std::atomic<bool> inference_v (true);
std::atomic<bool> screen_v (true);
std::atomic<bool> save_v (true);
std::atomic<bool> sequencer_v (true);


std::binary_semaphore capture1_semaphore (0);
std::binary_semaphore capture2_semaphore (0);
std::binary_semaphore inference_semaphore (0);
std::binary_semaphore screen_semaphore (0);
std::binary_semaphore save_semaphore (0);

bool system_init(){
    bool result;
    uint16_t col1;
    uint16_t col2;
    gpio_init();
    display_init();

    result = cam1.cam_init(1);
    if (!result){
        col1 = RED;
    }
    result = cam2.cam_init(2);
    if (!result){
        col2 = RED;
    }
    fill_screen(col1, col2);
    if (col1 == col2 and col1 == RED){
        return false;
    }else{
        result = yolo->load_model();
        if (!result){
            return false;
        }
    }
    return true;
}

void capture(Camera& cam, std::binary_semaphore& capture_semaphore){
    while (capture_v){
        capture_semaphore.acquire();
        if (cam.captureFrame()){
            Image img(&notif, nb);
            if(img.set_frame(cam.get_frame().clone(), cam.get_idDevice)){
                nb++;
                img_toAi.push(img);
            }
        }
    }
}

void inference(Ai* model){
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

void sequencer(){
    int i = 0;
    while (sequencer_v){
        capture1_semaphore.release();
        capture2_semaphore.release();
        inference_semaphore.release();
        save_semaphore.release();
        screen_semaphore.release();
    }
    usleep(60000);
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

void set_realtime_priority(int pr, int cpu) {
    struct sched_param param;
    param.sched_priority = pr;

    if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0) {
        notif.notice_err("Erreur : Impossible de definir les parametres de priorite")
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

int start(){
    std::thread cam1_thread ([](){
        set_realtime_priority(80, 1);
        capture(cam1, capture1_semaphore);
    });

    std::thread cam2_thread ([](){
        set_realtime_priority(80, 1);
        capture(cam2, capture2_semaphore);
    });

    std::thread inference_thread ([](){
        set_realtime_priority(99, 2);
        inference(&yolo);
    });

    std::thread screen_thread ([](){
        set_realtime_priority(80, 2);
        screen();
    });

    std::thread save_thread ([](){
        set_realtime_priority(70, 1);
        save();
    });

    std::thread sequencer_thread ([](){
        set_realtime_priority(98, 0);
        sequencer();
    });

    sequencer_thread.join();
}


int system_end();