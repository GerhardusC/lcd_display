#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
// #include "sdkconfig.h"
// #include "esp_system.h"
// #include "mqtt_client.h"

#include "shift_register.h"

// 0 command mode, 1 data mode
#define REGISTER_SELECT 25
// 0 write, 1 read
#define READ_WRITE 26
#define ENABLE_RW 27
#define BUSY_FLAG 35

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
        int bf = gpio_get_level(BUSY_FLAG);
        gpio_set_level(ENABLE_RW, 0);
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
    // Reset sequence
    ESP_LOGI("RESET", "RESET SEQUENCE");
    send_cmd(0b00111000);
    wait_us_blocking(10);
    send_cmd(0b00111000);
    wait_us_blocking(200);
    send_cmd(0b00111000);
    wait_us_blocking(80);

    ESP_LOGI("8_BIT_MODE", "Setting 8 bit mode.");
    // 8 bit mode
    send_cmd(0b00111000);
    wait_for_busy_flag();

    ESP_LOGI("CLEAR", "Clearing display");
    // Clear
    send_cmd(0b00000001);
    wait_for_busy_flag();

    ESP_LOGI("CURSOR_INCREMENT_MODE", "Changing mode of cursor to increment after each write.");
    // Entry mode increment
    send_cmd(0b00000110);
    wait_for_busy_flag();

    ESP_LOGI("DISPLAY ON", "Display on, cursor on, postion on");
    // Display on, cursor on, cursor postion on
    send_cmd(0b00001111);
    wait_for_busy_flag();
}

void app_main(void) {
    setup_shift_register();
    vTaskDelay(20);
    setup_screen();


    int i = 0;

    while(1){
        vTaskDelay(10);
        // push_u8_to_shift_register(0b11111111);
        write_data(0b11111111);
        vTaskDelay(10);
        if(i % 5 == 0){
            send_cmd(0b00000001);
        }
        i++;
        if(i > 100){
            i = 0;
        }
    }
}
