#include "System.hpp"

Notification notif("Plate_detection");
Camera cam1(&notif);
Camera cam2(&notif);
Ai* yolo = new Yolo_infer("Ai_file/model_quantified_executorch.pte", &notif, 0.25, 0.6);
int nb =0;
std::queue<Image> img_toAi;
std::queue<Image> img_toDisplay;
std::queue<Image> img_toSave;


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

bool capture(Camera& cam){
    if (cam.captureFrame()){
        Image img(&notif, nb);
        if(img.set_frame(cam.get_frame().clone(), cam.get_idDevice)){
            nb++;
            img_toAi.push(img);
            return true;
        }
    }
    return false;
}

bool inference(Ai* model){
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
            return true;
        }else{
            std::string msg = "No plate detection for img "+ img.get_idImage();
            notif.notice_info(msg);
            img_toSave.push(img);
            return true;
        }
    }
    return false;
}

void screen(){
    if (!img_toDisplay.empty()){
        auto img = img_toDisplay.pop();
        auto plate = img.get_plateFrame();
        if (!plate.empty()){
            fill_image(plate[0], img.get_camId());
        }
        
    }
}

void save(){
    if (!img_toSave.empty()){
        auto img = img_toSave.pop();
        img.save_frame();
        img.save_plateInfo();
    }
}

int start();
int system_end();