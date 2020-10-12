#include "utils.h"

/* Wrapper-Funktion for Serial-logging (with newline) */
void loggerNl(const char *str, const uint8_t logLevel) {
  if (serialDebug >= logLevel) {
    Serial.println(str);
  }
}

/* Wrapper-Funktion for Serial-Logging (without newline) */
void logger(const char *str, const uint8_t logLevel) {
  if (serialDebug >= logLevel) {
    Serial.print(str);
  }
}


int countChars(const char* string, char ch) {
    int count = 0;
    int length = strlen(string);

    for (uint8_t i = 0; i < length; i++) {
        if (string[i] == ch) {
            count++;
        }
    }

    return count;
}

bool isNumber(const char *str) {
  byte i = 0;

  while (*(str + i) != '\0') {
    if (!isdigit(*(str + i++))) {
      return false;
    }
  }

  if (i>0) {
      return true;
  } else {
      return false;
  }

}

// Checks if string starts with prefix
// Returns true if so
bool startsWith(const char *str, const char *pre) {
    if (strlen(pre) < 1) {
      return false;
    }

    return !strncmp(str, pre, strlen(pre));
}

// Checks if string ends with suffix
// Returns true if so
bool endsWith (const char *str, const char *suf) {
    const char *a = str + strlen(str);
    const char *b = suf + strlen(suf);

    while (a != str && b != suf) {
        if (*--a != *--b) break;
    }

    return b == suf && *a == *b;
}