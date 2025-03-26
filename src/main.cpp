#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include "HTTPClient.h"
#include <ArduinoJson.h>

#define SSID "WifiCadeau"
#define PASSWD "CadeauWifi"
#define URL "http://ilann-weather-api.glitch.me/Lyon"
#define RST_PIN         D6          // Configurable, see typical pin layout above
#define SS_PIN          D4       // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;

void setup() {
	Serial.begin(115200);
	while (!Serial);
	SPI.begin();
	mfrc522.PCD_Init();		// Init MFRC522

  // init wifi
  WiFi.begin(SSID, PASSWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not Connected");
    delay(100);
  }
  Serial.println("Connected");
}

void loop() {
  rfid_tag_present_prev = rfid_tag_present;

  _rfid_error_counter += 1;
  if(_rfid_error_counter > 2){
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

  if(result == mfrc522.STATUS_OK){
    if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue   
      return;
    }
    _rfid_error_counter = 0;
    _tag_found = true;        
  }
  
  rfid_tag_present = _tag_found;
  
  // rising edge
  if (rfid_tag_present && !rfid_tag_present_prev){
    Serial.println("Tag found");
    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));
  }
  
  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev){
    Serial.println("Tag gone");
  }
  delay(100);
}