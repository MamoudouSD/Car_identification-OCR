#ifndef SYSTEM_HPP
#define SYSTEM_HPP
#endif

#include "Display.hpp"
#include "Ai.hpp"
#include "Camera.hpp"
#include "Image.hpp"
#include "Notification.hpp"


extern Notification notif;
extern Camera cam1;
extern Camera cam2;
extern Ai* yolo = nullptr;
extern int nb;
extern std::queue<Image> img_toAi;
extern std::queue<Image> img_toDisplay;
extern std::queue<Image> img_toSave;


int system_init();
bool capture(Camera& cam);
bool inference(Ai* model);
void screen();
void save();
int start();
int system_end();