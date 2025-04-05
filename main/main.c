#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
// #include "mqtt_client.h"

#include "shift_register.h"

// 0 command mode, 1 data mode
#define REGISTER_SELECT 32
// 0 write, 1 read
#define READ_WRITE 33
#define ENABLE_RW 25

void setup_screen() {
    gpio_reset_pin(REGISTER_SELECT);
    gpio_reset_pin(READ_WRITE);
    gpio_reset_pin(ENABLE_RW);
    gpio_set_direction(REGISTER_SELECT, GPIO_MODE_OUTPUT);
    gpio_set_direction(READ_WRITE, GPIO_MODE_OUTPUT);
    gpio_set_direction(ENABLE_RW, GPIO_MODE_OUTPUT);
    gpio_set_level(REGISTER_SELECT, 0);
    gpio_set_level(ENABLE_RW, 0);
    push_u8_to_shift_register(0x01);
    gpio_set_level(READ_WRITE, 0);
    gpio_set_level(ENABLE_RW, 1);
    push_u8_to_shift_register(0x02);
    gpio_set_level(READ_WRITE, 0);
    gpio_set_level(ENABLE_RW, 1);
}

void writeword(uint8_t i) {
    gpio_set_level(ENABLE_RW, 0);
    gpio_set_level(REGISTER_SELECT, 1);
    push_u8_to_shift_register(i);
    gpio_set_level(ENABLE_RW, 1);
}



void app_main(void) {
    setup_shift_register();
    setup_screen();

    uint8_t i = 0;
    while(1){
        vTaskDelay(100);
        writeword(i);
        i++;
        if(i > 254){
            i = 0;
        }
    }

}