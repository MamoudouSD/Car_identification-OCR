#include "Display.hpp"
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>



namespace
{
    volatile uint32_t* gpio_base = nullptr;
    int gpio_fd = -1;

    constexpr size_t GPIO_BLOCK_SIZE = 4096;

    // offsets en mots 32 bits depuis gpio_base
    constexpr size_t GPFSEL0 = 0x00 / 4;
    constexpr size_t GPFSEL1 = 0x04 / 4;
    constexpr size_t GPFSEL2 = 0x08 / 4;
    constexpr size_t GPSET0  = 0x1C / 4;
    constexpr size_t GPCLR0  = 0x28 / 4;
    
    static inline void wr_strobe() {
        gpio_base[GPCLR0] = 0x200000;
        gpio_base[GPSET0] = 0x200000;
    }

    static inline void write_bus(uint32_t value) {
        gpio_base[GPCLR0] = 0x0FFFF0;
        gpio_base[GPSET0] = (value << 4);
    }

    static inline void write_data(uint32_t value) {
        write_bus(value);
        wr_strobe();
    }

    static inline void write_command(uint32_t cmd) {
        gpio_base[GPCLR0] = 0x500000;   // CS active, DC command
        write_bus(cmd);
        wr_strobe();
        gpio_base[GPSET0] = 0x100000;   // DC data
    }

    static inline void end_data_stream() {
        gpio_base[GPSET0] = 0x400000;
        gpio_base[GPSET0] = 0xF00000;
        gpio_base[GPCLR0] = 0x0FFFF0;
    }
} 

bool gpio_map_init() {
    gpio_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (gpio_fd < 0) {
        perror("open /dev/gpiomem");
        return false;
    }
    gpio_base = (volatile uint32_t*)(
        mmap(nullptr, GPIO_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpio_fd, 0)
    );
    if (gpio_base == MAP_FAILED) {
        perror("mmap");
        gpio_base = nullptr;
        close(gpio_fd);
        gpio_fd = -1;
        return false;
    }
    return true;
}

void gpio_map_close() {
    if (gpio_base) {
        munmap((uint32_t*)((const uint32_t*)(gpio_base)), GPIO_BLOCK_SIZE);
        gpio_base = nullptr;
    }
    if (gpio_fd >= 0) {
        close(gpio_fd);
        gpio_fd = -1;
    }
}

void gpio_init(){
    bool result =  gpio_map_init();
    if (gpio_base) {
        gpio_base[GPFSEL0] = GPFSEL0_V;
        gpio_base[GPFSEL1] = GPFSEL1_v;
        gpio_base[GPFSEL2] = GPFSEL2_v;
        gpio_base[GPSET0] = 0xF00000;
    }

}

void send_configData(uint32_t cmd, uint32_t *data, int length){
    write_command(cmd);
    for (int i=0; i <length; i++){
        write_data(data[i]);
    }
    end_data_stream();
}


void display_init(){
    uint32_t noData[0];
    gpio_base[GPCLR0] = 0x800000;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    gpio_base[GPSET0] = 0x800000;
    std::this_thread::sleep_for(std::chrono::milliseconds(150));


    uint32_t d[1] = {PARAM_MEMACCCTRL};
    send_configData(CMD_MEMACCCTRL, d, 1);

    uint32_t data_pag[4] = {0x0000, 0x0000, 0x0001, 0x003F};
    send_configData(CMD_PAGADSET, data_pag, 4);

    uint32_t posGamma[] = {0x0004, 0x0007, 0x0010, 0x0028, 0x0036, 0x0044, 0x0052, 0x0060, 0x006C, 0x0078, 0x008C, 0x009E, 0x00BB, 0x00D2, 0x00E5};
    send_configData(CMD_POSGAMMA, posGamma, sizeof(posGamma) / sizeof(posGamma[0]));

    uint32_t negGamma[] = {0x00E5, 0x00D2, 0x00BB, 0x009E, 0x008C, 0x0078, 0x006C, 0x0060, 0x0052, 0x0044, 0x0036, 0x0028, 0x0010, 0x0007, 0x0004};
    send_configData(CMD_NEGGAMMA, negGamma, sizeof(negGamma) / sizeof(negGamma[0]));

    d[0] = PARAM_INTERFPIXFORM;
    send_configData(CMD_INTERFPIXFORM, d, 1);

    d[0] = PARAM_BRIGHDISP;
    send_configData(CMD_BRIGHDISP, d, 1);

    send_configData(CMD_SLEEPOUT, noData, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    send_configData(CMD_DISPLAYON, noData, 0);

    fill_screen(RED, GREEN);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    fill_screen(GREEN, BLUE);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    fill_screen(BLUE, RED);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    fill_screen(GREEN, GREEN);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

}

void fill_color(uint32_t color){
    write_command(CMD_MEMWRITE);

    int length = LCD_WIDTH * LCD_HEIGHT;
    for (int i=0; i <length; i++){
        write_data(color);
    }

    end_data_stream();
}

void fill_screen(uint32_t color1, uint32_t color2){
    //fill part one
    uint32_t data_col1[4] = {0x0000, 0x0000, 0x0000, 0x00EF};
    send_configData(CMD_COLADSET, data_col1, 4);
    fill_color(color1);

    //fill part two
    uint32_t data_col2[4] = {0x0000, 0x00F0, 0x0001, 0x00DF};
    send_configData(CMD_COLADSET, data_col2, 4);
    fill_color(color2);
}

void fill_image(cv::Mat image, int part){
    uint16_t pixel;

    if (part == 1){
        uint32_t data_col1[4] = {0x0000, 0x0000, 0x0000, 0x00EF};
        send_configData(CMD_COLADSET, data_col1, 4);
    }else{
        uint32_t data_col2[4] = {0x0000, 0x00F0, 0x0001, 0x00DF};
        send_configData(CMD_COLADSET, data_col2, 4);
    }

    write_command(CMD_MEMWRITE);
    for (int y = 0; y < image.rows; y++) {
        const cv::Vec3b* row = image.ptr<cv::Vec3b>(y);
        for (int x = 0; x < image.cols; x++) {
            pixel = ((row[x][0] & 0xF8)<< 8) | ((row[x][1] & 0xFC)<<3) | (row[x][2]>>3);
            write_data(pixel);
        }
    }
    end_data_stream();
}

