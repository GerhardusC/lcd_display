#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "sdkconfig.h"
// #include "esp_system.h"
// #include "mqtt_client.h"

#include "lcd_screen.h"
#include "shift_register.h"

void app_main(void) {
    setup_shift_register();
    vTaskDelay(20);
    setup_screen();


    int i = 0;

    while(1){
        vTaskDelay(20);
        if(i % 15 == 0){
            send_cmd(0b00000001);
        } else {
            write_data(0b11111111);
        }
        i++;
        if(i > 100){
            i = 0;
        }
    }
}
