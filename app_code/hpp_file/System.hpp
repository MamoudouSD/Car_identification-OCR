#ifndef SYSTEM_HPP
#define SYSTEM_HPP
#endif

#include "Display.hpp"
#include "Ai.hpp"
#include "Camera.hpp"
#include "Image.hpp"
#include "Notification.hpp"
#include <queue>
#include <atomic>
#include <semaphore>
#include <thread>
#include <pthread.h>
#include "ThreadSafe_queue.hpp"




bool system_init();
void capture(Camera& cam);
void inference(Ai* model);
void screen();
void save();
void sequencer();
int start();
int system_end();