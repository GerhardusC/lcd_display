#include "driver/gpio.h"
#include "esp_timer.h"
#include "shift_register.h"
#include "esp_log.h"

void send_cmd(int cmd);
void write_data(uint8_t data);
void setup_screen();
