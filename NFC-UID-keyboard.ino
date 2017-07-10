/*
 * Reads a card UID from an ISO14443 card and outputs the UID as keyboard input.
 * 
 * Author: Søren Thing <soeren@thing.dk>
 * Released under MIT license - please see the file LICENSE.
 * 
 * HW:
 *  - A 3.3V/8MHz Arduino Pro Micro clone
 *  - A RFID-RC522 card reader (Buy at eBay or AliExpress)
 *  - A buzzer
 *  - An LED and a 180 ohm resistor
 *  - A plastic box (I used a 3.34"L x 1.96"W x 0.83"H one: https://www.aliexpress.com/item/J34-Free-Shipping-New-Plastic-Electronics-Project-Box-Enclosure-Case-DIY-3-34-L-x-1/32599725524.html)
 * 
 * Beware: The card reader is NOT 5V tolerant.
 * 
 * Connections:
 * Signal     Pin                    Pin
 *            Arduino Pro Micro      MFRC522 board
 * -----------------------------------------
 * Reset      10                     RST
 * SPI SS     18 (marked A0)         SDA
 * SPI MOSI   16                     MOSI
 * SPI MISO   14                     MISO
 * SPI SCK    15                     SCK
 * LED         6
 * Buzzer      9
 */
#include <SPI.h>
#include <MFRC522.h>
#include "Keyboard.h"

#define BUZZER_PIN 9
#define LED_PIN 6
#define SS_PIN A0
#define RST_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.
MFRC522::Uid lastUid;     // UID of last scanned card
unsigned long lastMillis; // value of millis() when last card was scanned.

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Keyboard.begin();
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();	// Init MFRC522 card
}

void loop() {
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  // Check that it is not a bounce of the same card
  if (memcmp(&lastUid, &mfrc522.uid, mfrc522.uid.size) == 0 // Same UID
        && (millis() - lastMillis) < 1000) { // Scanned again within on second 
    // Halt the card so it does not respond again while in the field
    mfrc522.PICC_HaltA();
    return;
  }

  // Turn on buzzer and LED
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);

  // Remember the time
  lastMillis = millis();
  
	// Output the UID followed by Enter.
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Keyboard.print(mfrc522.uid.uidByte[i] >> 4, HEX);
    Keyboard.print(mfrc522.uid.uidByte[i] & 0x0F, HEX);
  } 
  Keyboard.write(KEY_RETURN);

  // Wait a short while before turning off buzzer
  while (millis() < lastMillis + 75) {
    // Burn, baby, burn!
  }
  digitalWrite(BUZZER_PIN, LOW);

  // Wait a little before turning off LED
  while (millis() < lastMillis + 200) {
    // Burn, baby, burn!
  }
  digitalWrite(LED_PIN, LOW);

  // Halt the card so it does not respond again while in the field
  mfrc522.PICC_HaltA();

  // Remember the UID
  memcpy(&lastUid, &mfrc522.uid, sizeof(lastUid));
}

