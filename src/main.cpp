#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include "HTTPClient.h"
#include <ArduinoJson.h>

#define SSID "WifiCadeau"
#define PASSWD "CadeauWifi"
#define URL "http://guardia-api.iadjedj.ovh/unsecure/docs#/"  // 
#define RST_PIN         D6          // Configurable, see typical pin layout above
#define SS_PIN          D4       // Configurable, see typical pin layout above
#define GLED            D2
#define RLED            D1

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;

uint64_t convertUIDToInt(const uint8_t *uid) {
  uint64_t uidValue = 0;
  for (int i = 0; i < 8; i++) {
      uidValue = (uidValue << 8) | uid[i];
  }
  return uidValue;
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

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);
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
    uint64_t uid = convertUIDToInt(mfrc522.uid.uidByte);
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    Serial.println(uid);
    blinkgreen(3);
    
  }

  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev)
  {
    Serial.println("Tag gone");
  }
  delay(100);
}