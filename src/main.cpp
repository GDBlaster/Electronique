#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include "HTTPClient.h"
#include <ArduinoJson.h>

#define SSID "RouteurCadeau"
#define PASSWD "CadeauRouteur"
#define URL "10.1.143.107:3000" //
#define APITOKEN "khgyuikjhgytujnbhgtyuijh"
#define RST_PIN D6 // Configurable, see typical pin layout above
#define SS_PIN D4  // Configurable, see typical pin layout above
#define GLED D2
#define RLED D1

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

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

void api(String fin_url, String id)
{
  Serial.println(URL + fin_url + id);
  HTTPClient http;
  String resp;
  String full_url = String(URL) + fin_url;

  JsonDocument doc;
  doc["token"] = APITOKEN;
  doc["id"] = id;


  String jsonPayload;
  serializeJson(doc, jsonPayload);

  Serial.println(jsonPayload);

  http.begin(full_url);
  http.addHeader("Content-Type", "application/json");

  int code = http.sendRequest("GET", jsonPayload);

  if (code == HTTP_CODE_OK)
  {
    resp = http.getString();

    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, resp);

    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    const char *level = responseDoc["level"]; // "unauthorized", "user", "admin"
    http.end();

    if (strcmp(level, "user") == 0 || strcmp(level, "admin") == 0)
    {
      Serial.println("Passage autorisée");
      blinkgreen(3);
    }
    else
    {
      Serial.println(level);
      blinkred(3);
    }
  }
  else
  {
    Serial.println("Badge non connu");
    blinkred(3);
  }

  http.end(); // Close connection
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);

  WiFi.begin(SSID, PASSWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("WiFi Error");
  }
  Serial.println("WiFi connecté !");
}

void loop()
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

  // rising edge
  if (rfid_tag_present && !rfid_tag_present_prev)
  {
    Serial.println("Tag found");
    String uid = getUIDDecimal(mfrc522);
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    Serial.println(uid);
    api("/check", uid);
  }

  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev)
  {
    Serial.println("Tag gone");
  }
  delay(100);
}