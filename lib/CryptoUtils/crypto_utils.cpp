#include "crypto_utils.h"

void mac_address(uint8_t *key) {
    uint8_t mac[6];
    WiFi.macAddress(mac);

    // Compléter les 32 octets avec l'adresse MAC répétée
    for (int i = 0; i < 32; i++) {
        key[i] = mac[i % 6];
    }
}

String encrypt(const char *data) {
    uint8_t key[32];
    uint8_t iv[16] = {0};  
    char plaintext[256];
    char encrypted[256];

    mac_address(key);  

    memset(plaintext, 0, sizeof(plaintext));
    strcpy(plaintext, data);

    esp_aes_context ctx;
    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, key, 256);

    esp_aes_crypt_cbc(&ctx, ESP_AES_ENCRYPT, sizeof(plaintext), iv, (uint8_t *)plaintext, (uint8_t *)encrypted);
    esp_aes_free(&ctx);

    return String(encrypted);
}

String decrypt(String input_text, int size) {
    uint8_t key[32];
    uint8_t iv[16] = {0};  
    uint8_t encrypted_text[size];
    char result[size + 1];

    mac_address(key);  

    memcpy(encrypted_text, input_text.c_str(), input_text.length());

    esp_aes_context ctx;
    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, key, 256);
    esp_aes_crypt_cbc(&ctx, ESP_AES_DECRYPT, size, iv, encrypted_text, (uint8_t *)result);
    esp_aes_free(&ctx);

    result[size] = '\0';  // Assurer la fin de chaîne pour affichage correct
    return String(result);
}