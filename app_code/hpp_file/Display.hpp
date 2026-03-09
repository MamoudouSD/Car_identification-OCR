#ifndef DISPLAY_HPP
#define DISPLAY_HPP
#endif

#include <stdint.h>
#include <chrono>
#include <thread>
#include <opencv2/opencv.hpp>



#define GPFSEL0     (uint32_t *)0x7e200000
#define GPFSEL0_V   (uint32_t)0x36DB6000

#define GPFSEL1     (uint32_t *)0x7e200004
#define GPFSEL1_v     (uint32_t)0x36DB6DB6

#define GPFSEL2     (uint32_t *)0x7e200008
#define GPFSEL2_v     (uint32_t)0x249

#define GPSET0      (uint32_t *)0x720001C
#define GPCLR0      (uint32_t *)0x7200028





#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CMD_MEMWRITE ((uint32_t)0x002C)
#define CMD_SOFTRESET ((uint32_t)0x0001)
#define CMD_SLEEPOUT ((uint32_t)0x0011)
#define CMD_NORMDISPON ((uint32_t)0x0013)
#define CMD_DISPLAYON ((uint32_t)0x0029)
#define CMD_MEMACCCTRL ((uint32_t)0x0036)
#define PARAM_MEMACCCTRL ((uint32_t)0x0028)
#define CMD_INTERFPIXFORM ((uint32_t)0x003A)
#define PARAM_INTERFPIXFORM ((uint32_t)0x0055)
#define CMD_BRIGHDISP ((uint32_t)0x0051)
#define PARAM_BRIGHDISP ((uint32_t)0x0000)

#define CMD_COLADSET ((uint32_t)0x002A)
#define CMD_PAGADSET ((uint32_t)0x002B)

#define CMD_POWCTRL1 ((uint32_t)0x0017)
#define PARAM_POWCTRL1 ((uint32_t)0x00C0)

#define CMD_POWCTRL2 ((uint32_t)0x0041)
#define PARAM_POWCTRL2 ((uint32_t)0x00C1)

#define CMD_POSGAMMA ((uint32_t)0x00E0)
#define CMD_NEGGAMMA ((uint32_t)0x00E1)

#define CMD_VCOMCTRL ((uint32_t)0x0000)
#define PARAM_VCOMCTRL ((uint32_t)0x00C5)

#define CMD_INTERFMODECTRL ((uint32_t)0x00B0)
#define PARAM_INTERFMODECTRL ((uint32_t)0x0080)

static inline void wr_strobe();
static inline void write_bus(uint32_t value);
static inline void write_data(uint32_t value);
static inline void write_command(uint32_t cmd);
static inline void end_data_stream();
void gpio_init();
void send_configData(uint32_t cmd, uint32_t *data, int length);
void display_init();
void fill_color(uint32_t color);
void fill_screen(uint32_t color1, uint32_t color2);

void fill_image(cv::Mat image, int part);

