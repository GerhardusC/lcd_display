#include "lcd_screen.h"
#include "freertos/idf_additions.h"
#include <stdint.h>

// 0 command mode, 1 data mode
#define REGISTER_SELECT         25
// 0 write, 1 read
#define READ_WRITE              26
#define ENABLE_RW               27
#define BUSY_FLAG               35

// Commands:
#define SET_MODE_8_BIT          0b00111000
#define CLEAR                   0b00000001
#define DISPLAY_ON              0b00001100
#define DISPLAY_ON_WITH_CURSOR  0b00001111
#define CURSOR_INCREMENT_MODE   0b00000110
#define CURSOR_BOTTOM_LINE      0b11000000
#define CURSOR_TOP_LINE         0b01000000
#define RETURN_HOME             0b00000010
#define SHIFT_CURSOR_RIGHT      0b00010100
#define SHIFT_CURSOR_LEFT       0b00010000

#define STD_MAX_EXECUTION_TIME  37

void wait_us_blocking(uint32_t micros_to_wait) {
    uint64_t micros_now_plus_delay = esp_timer_get_time() + micros_to_wait;
    while(micros_now_plus_delay > esp_timer_get_time()){}
}

void wait_for_busy_flag() {
    push_u8_to_shift_register(0);
    gpio_set_level(READ_WRITE, 1);
    gpio_set_level(REGISTER_SELECT, 0);
    // Enable RW.
    int timeout = 0;

    while(1){
        gpio_set_level(ENABLE_RW, 1);
        wait_us_blocking(1);
        gpio_set_level(ENABLE_RW, 0);
        wait_us_blocking(1);
        int bf = gpio_get_level(BUSY_FLAG);
        if(bf == 0){
            return;   
        }
        timeout++;
        if(timeout > 1600){
            ESP_LOGE("WAITING_BUSY_FLAG", "TIMEOUT waiting for busy flag");
            return;
        }
    }
}

void send_cmd(int cmd) {
    gpio_set_level(ENABLE_RW, 0);
    // Push number to reg
    push_u8_to_shift_register(cmd);
    gpio_set_level(REGISTER_SELECT, 0);
    gpio_set_level(READ_WRITE, 0);

    wait_us_blocking(1);
    gpio_set_level(ENABLE_RW, 1);
    wait_us_blocking(1);
    // Disable RW again when no longer busy.
    gpio_set_level(ENABLE_RW, 0);
}

void write_data(uint8_t data){
    gpio_set_level(ENABLE_RW, 0);
    push_u8_to_shift_register(data);
    gpio_set_level(REGISTER_SELECT, 1);
    gpio_set_level(READ_WRITE, 0);

    wait_us_blocking(1);
    gpio_set_level(ENABLE_RW, 1);
    wait_us_blocking(1);
    gpio_set_level(ENABLE_RW, 0);
    wait_us_blocking(STD_MAX_EXECUTION_TIME);
}

void write_string(char *string) {
    bool string_done = false;

    send_cmd(CLEAR);
    vTaskDelay(2);

    // Write top row full of chars.
    for(uint8_t i = 0; i < 16; i++){
        if(string[i] == '\0'){
            string_done = true;
        }
        string_done ? write_data((uint8_t) ' ') : write_data((uint8_t) string[i]);
    }

    send_cmd(CURSOR_BOTTOM_LINE);
    wait_us_blocking(STD_MAX_EXECUTION_TIME);

    // Write bottom row full of chars.
    for(uint8_t i = 16; i < 32; i++){
        if(string[i] == '\0'){
            string_done = true;
        }
        string_done ? write_data((uint8_t) ' ') : write_data((uint8_t) string[i]);
    }

    send_cmd(CURSOR_TOP_LINE);
    vTaskDelay(1);
}


void write_one_line(enum Line16x2 line, char *string, uint8_t len) {
    uint8_t length = len - 1;
    send_cmd(RETURN_HOME);
    vTaskDelay(1);

    if(line == BOTTOM){
        for(uint8_t i = 0; i < 40; i++){
            send_cmd(SHIFT_CURSOR_RIGHT);
            wait_us_blocking(37);
        }
    }

    for(uint8_t i = 0; i < 16; i++){
        i > length ? write_data((uint8_t) ' ') : write_data((uint8_t) string[i]);
    }
    vTaskDelay(1);
}

void setup_screen() {
    int pins[] = {
        REGISTER_SELECT,
        BUSY_FLAG,
        READ_WRITE,
        ENABLE_RW,
    };
    for(int i = 0; i < 4; i++){
        gpio_reset_pin(pins[i]);
        if(pins[i] == BUSY_FLAG){
            gpio_set_direction(pins[i], GPIO_MODE_INPUT);
        } else {
            gpio_set_direction(pins[i], GPIO_MODE_OUTPUT);
            gpio_set_level(pins[i], 0);
        }
    }
    //
    // Reset sequence.
    //
    ESP_LOGI("RESET", "RESET SEQUENCE");
    send_cmd(SET_MODE_8_BIT);
    wait_us_blocking(10);
    send_cmd(SET_MODE_8_BIT);
    wait_us_blocking(200);
    send_cmd(SET_MODE_8_BIT);
    wait_us_blocking(80);

    ESP_LOGI("8_BIT_MODE", "Setting 8 bit mode.");
    send_cmd(SET_MODE_8_BIT);
    wait_for_busy_flag();

    // These just happen by default, but can be explicit if needed.
    //
    ESP_LOGI("CLEAR", "Clearing display");
    send_cmd(CLEAR);
    vTaskDelay(1);

    ESP_LOGI("CURSOR_INCREMENT_MODE", "Changing mode of cursor to increment after each write.");
    send_cmd(CURSOR_INCREMENT_MODE);
    wait_us_blocking(37);
    //
    ESP_LOGI("DISPLAY ON", "Display on, cursor off, postion off");
    send_cmd(DISPLAY_ON);
    wait_us_blocking(37);
}
