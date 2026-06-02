#include <Wire.h>
#include <Adafruit_PN532.h>

Adafruit_PN532 nfc(255,255,&Wire);

void setup() {
  Serial.begin(115200);

  Wire.begin(13,15);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  if (!versiondata) {
    Serial.println("PN532 not found");
    while(1);
  }

  Serial.println("Waiting for tag...");

  nfc.SAMConfig();
}

void loop() {

  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength)) {

    Serial.print("Tag found: ");

    for(int i=0;i<uidLength;i++) {
      Serial.print(uid[i],HEX);
      Serial.print(" ");
    }

    Serial.println();

    delay(1000);
  }
}