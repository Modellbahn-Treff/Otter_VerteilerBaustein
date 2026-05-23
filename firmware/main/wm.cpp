#include "driver/gpio.h"
#include "esp_log.h"
#include "wm.h"
#include "otter.h"
#include "settings.h"
#include <string.h>

static const char *TAG = "WM";

bool AmSchalten[8]  = {1};
int  AusSchaltZeit[8] = {1};

void WM_Trigger(uint8_t number, const char *richtung) {
    if (strcmp(richtung, "0") == 0) {
        ESP_LOGI(TAG, "Weiche %u auf '0' geschaltet.", number);
        gpio_set_level((gpio_num_t)Pin[2 * number],     0);
        gpio_set_level((gpio_num_t)Pin[2 * number + 1], 1);
        AmSchalten[number]   = 1;
        AusSchaltZeit[number] = AusSchaltZeitWeiche;
    } else if (strcmp(richtung, "1") == 0) {
        ESP_LOGI(TAG, "Weiche %u auf '1' geschaltet.", number);
        gpio_set_level((gpio_num_t)Pin[2 * number],     1);
        gpio_set_level((gpio_num_t)Pin[2 * number + 1], 0);
        AmSchalten[number]   = 1;
        AusSchaltZeit[number] = AusSchaltZeitWeiche;
    } else {
        ESP_LOGW(TAG, "Richtung existiert nicht");
    }
}

void WM_Loop(uint8_t number) {
    if (AmSchalten[number] == 1) {
        AusSchaltZeit[number]--;
        if (AusSchaltZeit[number] <= 0) {
            AmSchalten[number] = 0;
            gpio_set_level((gpio_num_t)Pin[2 * number],     0);
            gpio_set_level((gpio_num_t)Pin[2 * number + 1], 0);
            ESP_LOGI(TAG, "W%u aus", number);
        }
    }
}

void WM_Start(uint8_t number) {
    gpio_set_direction((gpio_num_t)Pin[2 * number],     GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)Pin[2 * number + 1], GPIO_MODE_OUTPUT);
}
