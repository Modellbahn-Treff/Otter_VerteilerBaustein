#include "driver/gpio.h"
#include "tm.h"
#include "otter.h"
#include "settings.h"

uint8_t TM_LastSentValue[20] = {2};

void TM_Refresh(uint8_t number) {
    for (uint8_t i = 0; i < 20; i++) {
        TM_LastSentValue[i] = 2;
    }
}

void TM_Loop(uint8_t number) {
    uint8_t level = gpio_get_level((gpio_num_t)Pin[number]);
    if (level == TM_LastSentValue[number]) return;
    esp_mqtt_client_publish(mqtt_client, MqttTMT[number], level ? "1" : "0", 1, 0, 0);
    TM_LastSentValue[number] = level;
}

void TM_Start(uint8_t number) {
    gpio_set_direction((gpio_num_t)Pin[number], GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)Pin[number], GPIO_PULLUP_ONLY);
}
