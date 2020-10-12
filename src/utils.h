#ifndef H_UTILS
#define H_UTILS

#include "consts.h"
#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy

void loggerNl(const char *str, const uint8_t logLevel);
void logger(const char *str, const uint8_t logLevel);

bool isNumber(const char *str);
bool endsWith (const char *str, const char *suf);
bool startsWith(const char *str, const char *pre);

#endif