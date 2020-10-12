#ifndef H_LED
#define H_LED

#include "consts.h"
#include <stdint.h>

#include "esp_task_wdt.h"
#include <FastLED.h>

extern bool showLedError;
extern bool showLedOk;
extern bool showPlaylistProgress;
extern bool showRewind;

extern uint8_t initialLedBrightness; // Initial brightness of Neopixel
extern uint8_t ledBrightness;
extern uint8_t nightLedBrightness; // Brightness of Neopixel in nightmode

void showLed(void *parameter);

#endif