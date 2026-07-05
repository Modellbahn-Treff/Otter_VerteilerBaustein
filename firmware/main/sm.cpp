// Copyright (C) 2026 Modellbahn-Treff for Kids GmbH
// SPDX-License-Identifier: GPL-3.0-or-later

#include "driver/gpio.h"
#include "esp_log.h"
#include "sm.h"
#include "otter.h"
#include <string.h>

static const char *TAG = "SM";

void SM_Trigger(uint8_t number, const char *farbe) {
    if (strcmp(farbe, "red") == 0) {
        ESP_LOGI(TAG, "Signal %u auf 'red' geschaltet.", number);
        gpio_set_level((gpio_num_t)Pin[2 * number],     0);
        gpio_set_level((gpio_num_t)Pin[2 * number + 1], 1);
    } else if (strcmp(farbe, "green") == 0) {
        ESP_LOGI(TAG, "Signal %u auf 'green' geschaltet.", number);
        gpio_set_level((gpio_num_t)Pin[2 * number],     1);
        gpio_set_level((gpio_num_t)Pin[2 * number + 1], 0);
    } else {
        ESP_LOGW(TAG, "Farbe existiert nicht");
    }
}

void SM_Start(uint8_t number) {
    gpio_set_direction((gpio_num_t)Pin[2 * number],     GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)Pin[2 * number + 1], GPIO_MODE_OUTPUT);
}
