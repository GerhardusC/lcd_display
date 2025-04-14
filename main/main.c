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

    char* lines[32] = {
        //              ||
        "Once upon a time ",
        //              ||
        "In a galaxy far,far away...",
        //              ||
        "Someone...      for some reason",
        //              ||
        "wrote code for  the LCD-16x2",
        //              ||
    };


    int i = 0;

    while(1){
        write_string(lines[i%4]);
        vTaskDelay(1000);
        i++;
        if(i > 100){
            i = 0;
        }
    }
}
