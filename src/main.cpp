#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include <WiFiClientSecure.h>
#include "HTTPClient.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ArduinoNvs.h>
#include <aes/esp_aes.h>
#include <string.h>
#include "time.h"

#define RST_PIN D6 // Configurable, see typical pin layout above
#define SS_PIN D4  // Configurable, see typical pin layout above
#define GLED D2
#define RLED D1

// INITIALISATION DES VARIABLES | TABLEAUX | INSTANCES :

MFRC522 mfrc522(SS_PIN, RST_PIN);
Preferences preferences;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;     // UTC+1 (fuseau horaire)
const int daylightOffset_sec = 3600; // heure d'été

unsigned long lastTimeCheck = millis();
const long interval = 60000;
bool lecturebadge = false;

bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;

String getUIDDecimal(MFRC522 &mfrc522)
{
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    uidString += String(mfrc522.uid.uidByte[i], DEC);
  }
  return uidString;
}

// FAIT CLIGNOTER LES LED
void Blink(int count, int led)
{
  int i = 0;
  while (i < count)
  {
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);
    i++;
  }
}

void blinkgreen(int count)
{
  Blink(count, GLED);
}

void blinkred(int count)
{
  Blink(count, RLED);
}

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

    //Serial.printf("Encrypted text: %s\n", encrypted);
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
    //Serial.printf("Decrypted text: %s\n", result);
    return String(result);
}

void initialisation()
{
  String jwt_enc = encrypt("khgyuikjhgytujnbhgtyuijh");
  String ssid_enc = encrypt("Suu");
  String passwd_enc = encrypt("popallec");
  String url_enc = encrypt("http://192.168.253.23:3000/check");
  Serial.println("url enc");
  Serial.println(url_enc);

  preferences.begin("myApp", false);
  preferences.putString("token", jwt_enc);
  preferences.putString("SSID", ssid_enc);
  preferences.putString("PASSWD", passwd_enc);
  preferences.putString("URL", url_enc);
  preferences.end();
}

// CREATION DES STRUCTURE  POUR RECUP INFO DE NVS
struct Credentials
{
  String ssid;
  String passwd;
};

struct Credentials_jwt
{
  String url;
  String jwt;
};
// RECUPERATION DES INFORMATION DE NVS
Credentials get_from_nvs_credit()
{
  preferences.begin("myApp", false);
  String ssid = preferences.getString("SSID", "pas_de_token");
  String passwd = preferences.getString("PASSWD", "pas_de_token");
  preferences.end();

  return {decrypt(ssid, 256), decrypt(passwd, 256)};
}

Credentials_jwt get_from_nvs_url_jwt()
{
  preferences.begin("myApp", false);
  String token = preferences.getString("token", "pas_de_token");
  String url = preferences.getString("URL", "pas_de_token");
  preferences.end();

  return {decrypt(url, 256), decrypt(token, 256)};
}

void api(String id)
{
  Credentials_jwt creds = get_from_nvs_url_jwt();
  Serial.println(creds.url + id);
  HTTPClient http;
  String resp;
  String full_url = String(creds.url);

  JsonDocument doc;
  doc["token"] = creds.jwt;
  doc["id"] = id;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  Serial.println(jsonPayload);

  http.begin(full_url);
  http.addHeader("Content-Type", "application/json");

  int code = http.sendRequest("POST", jsonPayload);

  Serial.println(code);

  if (code == HTTP_CODE_OK)
  {
    resp = http.getString();
    Serial.println("Réponse reçue : " + resp);

    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, resp);

    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    const char* permission = responseDoc["permission"]; // "unauthorized", "user", "admin"
    http.end();                       // connexion fermée

    Serial.println(permission);

    if (permission && (strcmp(permission, "user") == 0 || strcmp(permission, "admin") == 0))
    {
      Serial.println("✅ Passage autorisé !");
      blinkgreen(3);
    }
    else
    {
      Serial.println(permission);
      blinkred(3);
    }
  }
  else
  {
    Serial.println("Badge non connu");
    blinkred(3);
  }

  http.end(); // connexion fermée
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  //initialisation();
  lastTimeCheck = millis() - interval;
  Credentials creds = get_from_nvs_credit();
  
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);

  WiFi.begin(creds.ssid, creds.passwd);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("Erreur Wifi");
  }
  Serial.println("WiFi connecté !");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void checkTime()
{
  if (millis() - lastTimeCheck > interval)
  {                           // Vérification après 60 secondes
    lastTimeCheck = millis(); // Mise à jour du dernier temps de vérification
    Serial.println("DEBUG: Interval atteint, mise à jour de l'heure");

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Échec de l'obtention de l'heure");
      return;
    }

    int heure = timeinfo.tm_hour;
    Serial.printf("Heure actuelle : %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    if (heure >= 8 && heure < 19)
    {
      Serial.println("lecture de badge activée");
      lecturebadge = true;
    }
    else
    {
      Serial.println("lecture de badge désactivée");
      lecturebadge = false;
    }
  }
}

void loop()
{
  checkTime();
  

  if (lecturebadge == true)
  {
    rfid_tag_present_prev = rfid_tag_present;

    _rfid_error_counter += 1;
    if (_rfid_error_counter > 2)
    {
      _tag_found = false;
    }

    // Detect Tag without looking for collisions
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);

    // Reset baud rates
    mfrc522.PCD_WriteRegister(mfrc522.TxModeReg, 0x00);
    mfrc522.PCD_WriteRegister(mfrc522.RxModeReg, 0x00);
    // Reset ModWidthReg
    mfrc522.PCD_WriteRegister(mfrc522.ModWidthReg, 0x26);

    MFRC522::StatusCode result = mfrc522.PICC_RequestA(bufferATQA, &bufferSize);

    if (result == mfrc522.STATUS_OK)
    {
      if (!mfrc522.PICC_ReadCardSerial())
      { // Since a PICC placed get Serial and continue
        return;
      }
      _rfid_error_counter = 0;
      _tag_found = true;
    }

    rfid_tag_present = _tag_found;

    // si badge detectée
    if (rfid_tag_present && !rfid_tag_present_prev)
    {
      Serial.println("Tag found");
      String uid = getUIDDecimal(mfrc522);
      //  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
      Serial.println(uid);
      api(uid);
    }

    // si badge n'est plus detectée
    if (!rfid_tag_present && rfid_tag_present_prev)
    {
      Serial.println("Tag gone");
    }
    delay(100);
  }
  else
  {
    Serial.println("lecture de badge désactivée");
    delay(3000);
  }
}