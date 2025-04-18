#include <stdint.h>
#include <stdio.h>
#include "esp_event_base.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
// #include "sdkconfig.h"
// #include "esp_system.h"

#include "secret.h"
#include "lcd_screen.h"
#include "shift_register.h"
#include "wifi.h"

#define TAG "TEMP_AND_HUMIDITY"

bool check_topic(char *topic, char *expected_topic, int len) {
    for(int i = 0; i < len; i++){
        if(expected_topic[i] != topic[i]){
            return false;
            break;
        }
    }
    return true;
}

static void temp_collect_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(client, "/home/#", 0);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");

            if(check_topic(event->topic, "/home/temperature", event->topic_len)){
                char safe_event_data[] = {
                    event->data[0], event->data[1], event->data[2], event->data[3], 0b11011111, 'C'
                };
                write_one_line(TOP, safe_event_data, 6);
            }

            if(check_topic(event->topic, "/home/humidity", event->topic_len)){
                char safe_event_data[] = {
                    event->data[0], event->data[1], ' ', '%'
                };
                write_one_line(BOTTOM, safe_event_data, 4);
            }

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
        }
}

static esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = MY_BROKER_IP,
    .task = {
        .priority = 1,
        .stack_size = 3000,
    },
};

void app_main(void) {
    setup_shift_register();
    vTaskDelay(20);
    setup_screen();
    wifi_init_connection();

    esp_mqtt_client_handle_t mqtthandle =  esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtthandle, ESP_EVENT_ANY_ID, temp_collect_event_handler, NULL);
    esp_mqtt_client_start(mqtthandle);

    write_one_line(TOP, "Ready", 5);
    vTaskDelay(10);
    write_one_line(BOTTOM, "To go", 5);

    while(1){
        vTaskDelay(1000);
    }
}
