#include "Display.hpp"

uint32_t value;

void gpio_init(){
    *GPFSEL0 = GPFSEL0_V;
    *GPFSEL1 = GPFSEL1_v;
    *GPFSEL2 = GPFSEL2_v;
    *GPSET0 = 0xF00000;

}

void send_data(uint32_t cmd, uint32_t *data, int length){
    *GPCLR0 = 0x500000;

    *GPSET0 = (cmd << 4);

    *GPCLR0 = 0x200000;
    *GPSET0 = 0x200000;

    *GPSET0 = 0x100000;

    for (int i=0; i <length; i++){
        *GPCLR0 = 0x0FFFF0;
        *GPSET0 = (data[i]<<4);

        *GPCLR0 = 0x200000;
        *GPSET0 = 0x200000;
    }

    *GPSET0 = 0x400000;

    *GPSET0 = 0xF00000;
    *GPCLR0 = 0x0FFFF0;
}


void display_init(){
    uint32_t noData[0];
    *GPCLR0 = 0x800000;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    *GPSET0 = 0x800000;
    std::this_thread::sleep_for(std::chrono::milliseconds(150));


    uint32_t d[1] = {PARAM_MEMACCCTRL};
    send_data(CMD_MEMACCCTRL, d, 1);

    uint32_t data_pag[4] = {0x0000, 0x0000, 0x0001, 0x003F};
    send_data(CMD_PAGADSET, data_pag, 4);

    uint32_t posGamma[] = {0x0004, 0x0007, 0x0010, 0x0028, 0x0036, 0x0044, 0x0052, 0x0060, 0x006C, 0x0078, 0x008C, 0x009E, 0x00BB, 0x00D2, 0x00E5};
    send_data(CMD_POSGAMMA, posGamma, sizeof(posGamma) / sizeof(posGamma[0]));

    uint32_t negGamma[] = {0x00E5, 0x00D2, 0x00BB, 0x009E, 0x008C, 0x0078, 0x006C, 0x0060, 0x0052, 0x0044, 0x0036, 0x0028, 0x0010, 0x0007, 0x0004};
    send_data(CMD_NEGGAMMA, negGamma, sizeof(negGamma) / sizeof(negGamma[0]));

    d[0] = PARAM_INTERFPIXFORM;
    send_data(CMD_INTERFPIXFORM, d, 1);

    d[0] = PARAM_BRIGHDISP;
    send_data(CMD_BRIGHDISP, d, 1);

    send_data(CMD_SLEEPOUT, noData, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    send_data(CMD_DISPLAYON, noData, 0);

    fill_screen(RED, GREEN);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    fill_screen(GREEN, BLUE);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    fill_screen(BLUE, RED);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

}

void fill_color(uint32_t color){
    *GPCLR0 = 0x500000;

    *GPSET0 = (CMD_MEMWRITE << 4);

    *GPCLR0 = 0x200000;
    *GPSET0 = 0x200000;

    *GPSET0 = 0x100000;

    int length = LCD_WIDTH * LCD_HEIGHT;
    for (int i=0; i <length; i++){
        *GPCLR0 = 0x0FFFF0;
        *GPSET0 = (color<<4);

        *GPCLR0 = 0x200000;
        *GPSET0 = 0x200000;
    }

    *GPSET0 = 0x400000;

    *GPSET0 = 0xF00000;
    *GPCLR0 = 0x0FFFF0;
}

void fill_screen(uint32_t color1, uint32_t color2){
    //fill part one
    uint32_t data_col1[4] = {0x0000, 0x0000, 0x0000, 0x00EF};
    send_data(CMD_COLADSET, data_col1, 4);
    fill_color(color1);

    //fill part two
    uint32_t data_col2[4] = {0x0000, 0x00F0, 0x0001, 0x00DF};
    send_data(CMD_COLADSET, data_col2, 4);
    fill_color(color2);

}