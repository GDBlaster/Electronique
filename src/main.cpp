#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         6          // Configurable, see typical pin layout above
#define SS_PIN          4       // Configurable, see typical pin layout above
#define GLED            D1
#define RLED            D2

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;

void setup() {
	Serial.begin(115200);
  Serial.println("1");		// Initialize serial communications with the PC
	while (!Serial);
  Serial.println("2");		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();
  Serial.println("3");			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
  Serial.println("4");
}

void loop() {
  Serial.println("dans la boucle");
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
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  }
  
  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev){
    Serial.println("Tag gone");
  }
  delay(100);
}