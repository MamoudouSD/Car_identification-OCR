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
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>



bool system_init();
void capture(Camera& cam);
void inference(Ai* model);
void screen();
void save();
void sequencer();
int start();
void set_non_blocking(bool enable);
void watchdog();
int system_end();