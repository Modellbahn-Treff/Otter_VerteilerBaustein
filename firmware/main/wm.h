#pragma once
#include <stdint.h>
#include <stdbool.h>

extern bool AmSchalten[8];
extern int  AusSchaltZeit[8];

void WM_Trigger(uint8_t number, const char *richtung);
void WM_Loop(uint8_t number);
void WM_Start(uint8_t number);
