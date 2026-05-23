#pragma once
#include <stdint.h>
#include "mqtt_client.h"

extern esp_mqtt_client_handle_t mqtt_client;

void TM_Refresh(uint8_t number);
void TM_Loop(uint8_t number);
void TM_Start(uint8_t number);
