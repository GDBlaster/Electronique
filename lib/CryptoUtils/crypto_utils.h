#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include <aes/esp_aes.h>

void mac_address(uint8_t *key);
String encrypt(const char *data);
String decrypt(String input_text, int size);

#endif // CRYPTO_UTILS_H