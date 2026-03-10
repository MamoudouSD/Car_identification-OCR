#include "System.hpp"

Notification notif("Plate_detection");
Camera cam1(&notif);
Camera cam2(&notif);
Ai* yolo = new Yolo_infer("Ai_file/model_quantified_executorch.pte", &notif, 0.25, 0.6);


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
int start();
int system_end();